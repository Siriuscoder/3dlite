#ifndef LITE3D_BINDLESS_TEXTURE_PIPELINE

uniform samplerCube Environment;

#ifdef LITE3D_ENV_PROBE_MAX

uniform samplerCubeArray EnvironmentProbe;
uniform samplerCubeArray EnvironmentProbeDepth;

#endif
#endif

#ifdef LITE3D_ENV_PROBE_MAX

uniform sampler2D BrdfLUT;
layout(std430) readonly buffer EnvProbesData
{
    EnvironmentProbeStruct probes[LITE3D_ENV_PROBE_MAX];
};

#endif

vec3 IBLSpecularEnergy(vec3 F, float NdotV, in Surface surface)
{
#ifdef LITE3D_ENV_PROBE_MAX
    // Apply freshnel BRDF LUT correction only for environment probes
    const vec2 coef = texture(BrdfLUT, vec2(NdotV, surface.material.roughness)).rg;
    // fdez-aguera 2019 from the split sum terms, a + b is the single scatter albedo of a white lobe,
    // the energy it loses to microfacet shadowing comes back tinted by f0, rough metals stop going dark
    const float singleScatter = max(coef.x + coef.y, 0.05);
    const vec3 a1 = (surface.F0 * coef.x) + coef.y;
    const vec3 b1 = 1.0 + surface.F0 * (1.0 / singleScatter - 1.0); // MultiscatterEnergySplitSum
    return a1 * b1;
#else
    // Apply freshnel-Roughness fallback for environment images 
    return F;
#endif
}

float getEnvMaxLod(in Material material)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return log2(float(textureSize(material.environment.textureId, 0).x));
#else
    return log2(float(textureSize(Environment, 0).x));
#endif
}

vec3 sampleEnvLod(in Material material, vec3 uv, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(material.environment.textureId, uv * material.environmentUVScale, lod).rgb;
#else
    return textureLod(Environment, uv * material.environmentUVScale, lod).rgb;
#endif
}

void getProbeMaxLodAndCount(in Material material, inout int count, inout float maxLod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ivec3 sizes = textureSize(material.environmentProbe.textureId, 0);
#else
    ivec3 sizes = textureSize(EnvironmentProbe, 0);
#endif

    count = sizes.z;
    maxLod = log2(float(sizes.x));
}

vec3 sampleProbeLod(in Material material, vec3 uv, int index, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(material.environmentProbe.textureId, vec4(uv, index), lod).rgb;
#else
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
#endif
}

float sampleProbeDepth(in Material material, vec3 uv, int index)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return texture(material.environmentProbeDepth.textureId, vec4(uv, index)).r;
#else
    return texture(EnvironmentProbeDepth, vec4(uv, index)).r;
#endif
}

SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular)
{
    SufraceLighting result = SufraceLighting(vec3(0.0), vec3(0.0));
    float maxLod = 0.0;
    int probesCount = 0;

// В случае если запущен bindless пайплайн всегда декларируем эту секцию так как ее использование зависит от 
// флагов материала. Иначе движок выставляет LITE3D_ENABLE_ENVIRONMENT_TEXTURE если в пайплайне включено
// использование текстуры окружения
#if defined(LITE3D_BINDLESS_TEXTURE_PIPELINE) || defined(LITE3D_ENABLE_ENVIRONMENT_TEXTURE)
    if (hasFlag(surface.material.flags, LITE3D_MATERIAL_ENVIRONMENT_TEXTURE))
    {
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
        if (hasFlag(surface.material.environment.flags, TEXTURE_FLAG_LOADED))
#endif
        {
            maxLod = getEnvMaxLod(surface.material);
#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            // Duffuse irradiance 
            result.diffuse = sampleEnvLod(surface.material, surface.normal, maxLod - 1.0);
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            // Specular 
            float specularLevel = surface.material.roughness * max(maxLod - 1.0. 0.0);
            result.specular = sampleEnvLod(surface.material, angular.viewReflectDir, specularLevel);
#endif
        }
    }
#endif

#ifdef LITE3D_ENV_PROBE_MAX // The IBL is enabled in the engine 
    if (hasFlag(surface.material.flags, LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE))
    {
        getProbeMaxLodAndCount(surface.material, probesCount, maxLod);
        float specularLevel = surface.material.roughness * max(maxLod - 1.0, 0.0);
        float visClamped = max(surface.ao, 0.05); 

        if (surface.material.environmentSingleProbeIndex >= 0)
        {
            EnvironmentProbeStruct probe = probes[surface.material.environmentSingleProbeIndex];

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
            {
                result.diffuse += irradianceSHL2(surface.normal, probe.sh, visClamped);
            }
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
            {
                result.specular += sampleProbeLod(surface.material, angular.viewReflectDir, 
                    surface.material.environmentSingleProbeIndex, specularLevel);
            }
#endif
        }
        else
        {
            float nearProbeDistance = FLT_MAX;

            for (int p = 0; p < probesCount; ++p)
            {
                if (hasFlag(probes[p].flags, LITE3D_ENV_PROBE_FLAG_REUSABLE)) // Empty probe, may be reused
                    continue;

                float probeDistance = distance(surface.wv, probes[p].position.xyz);
                nearProbeDistance = min(nearProbeDistance, probeDistance);
            }

            // Calc indirect light
            float totalDWeight = FLT_EPSILON;
            float totalSWeight = FLT_EPSILON;
            vec3 totalSpecular = vec3(0.0);

            vec4 radianceSH[LITE3D_SH_COEFFS] = {
                vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0)
            };

            for (int p = 0; p < probesCount; ++p)
            {
                if (hasFlag(probes[p].flags, LITE3D_ENV_PROBE_FLAG_REUSABLE)) // Empty probe, may be reused
                    continue;

                EnvironmentProbeStruct probe = probes[p];
                float probeDistance = distance(surface.wv, probe.position.xyz);
                float relativeDistance = nearProbeDistance / max(probeDistance, FLT_EPSILON);

                if (relativeDistance < LITE3D_ENV_PROBE_RELATIVE_DISTANCE_THRESHOLD)
                    continue;

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
                {
                    float dW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_DIFFUSE_POWER), FLT_EPSILON);
                    // Interpolate SH coeffs between probes 
                    for (uint ir = 0; ir < LITE3D_SH_COEFFS; ++ir)
                    {
                        radianceSH[ir] += probe.sh[ir] * dW;
                    }

                    totalDWeight += dW;
                }
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
                {
                    float sW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_SPECULAR_POWER), FLT_EPSILON);
                    totalSpecular += sampleProbeLod(surface.material, R, p, specularLevel) * sW;
                    totalSWeight += sW;
                }
#endif
            }

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            for (uint ir = 0; ir < LITE3D_SH_COEFFS; ++ir)
            {
                radianceSH[ir] /= totalDWeight;
            }

            result.diffuse += irradianceSHL2(surface.normal, radianceSH, 1.0);
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            result.specular += totalSpecular / totalSWeight;
#endif
        }
    }
#endif

    vec3 F = fresnelSchlickRoughness(angular.NdotV, surface);
    vec3 specEnergy = IBLSpecularEnergy(F, angular.NdotV, surface);

    result.diffuse *= diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb * surface.material.envDiffuse;
    result.specular *= specEnergy * surface.material.envSpecular;

    return result;
}
