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
    return log2(float(textureSize(samplerCube(material.environment.textureId), 0).x));
#else
    return log2(float(textureSize(Environment, 0).x));
#endif
}

vec3 sampleEnvLod(in Material material, vec3 uv, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(samplerCube(material.environment.textureId), uv * material.environmentUVScale, lod).rgb;
#else
    return textureLod(Environment, uv * material.environmentUVScale, lod).rgb;
#endif
}

void getProbeMaxLodAndCount(in Material material, inout int count, inout float maxLod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ivec3 sizes = textureSize(samplerCubeArray(material.environmentProbe.textureId), 0);
#else
    ivec3 sizes = textureSize(EnvironmentProbe, 0);
#endif

    count = sizes.z;
    maxLod = log2(float(sizes.x));
}

vec3 sampleProbeLod(in Material material, vec3 uv, int index, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(samplerCubeArray(material.environmentProbe.textureId), vec4(uv, index), lod).rgb;
#else
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
#endif
}

float sampleProbeDepth(in Material material, vec3 uv, int index)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return texture(samplerCubeArray(material.environmentProbeDepth.textureId), vec4(uv, index)).r;
#else
    return texture(EnvironmentProbeDepth, vec4(uv, index)).r;
#endif
}

vec3 computeDominantSpecularDirection(in Surface surface, in AngularInfo angular)
{
    // frostbite/epic dominant direction - biases toward normal as roughness increases
    // this better represents where most specular energy comes from for rough surfaces
    float smoothness  = 1.0 - surface.material.roughness;
    float factor      = smoothness * (sqrt(smoothness) + surface.material.roughness);
    return normalize(mix(surface.normal, angular.viewReflectDir, factor));
}

float computeSpecularOcclusion(in Surface surface, in AngularInfo angular, vec3 dominantSpecularDirection)
{
    float exponent = exp2(-16.0 * surface.material.roughness - 1.0);

    float occlusion = clamp(pow(max(angular.NdotV + surface.ao, 1e-6), exponent) - 1.0 + surface.ao, 0.0, 1.0);
    float horizon = clamp(1.0 + dot(dominantSpecularDirection, surface.normal), 0.0, 1.0);

    return occlusion * horizon * horizon;
}

// approx of multi-bounce (inter-reflection) for ao
vec3 gtaoMultiBounce(float visibility, vec3 albedo)
{
    // Jimenez et al. 2016 approximation
    vec3 a = 2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c = 2.7552 * albedo + 0.6903;
    
    return max(vec3(visibility), ((visibility * a + b) * visibility + c) * visibility);
}

SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular)
{
    SufraceLighting result = SufraceLighting(vec3(0.0), vec3(0.0), vec3(0.0));
    float maxLod = 0.0;
    int probesCount = 0;
    float visibility = max(surface.ao, 0.1); 

    vec3 dominant = computeDominantSpecularDirection(surface, angular);
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
            float specularLevel = surface.material.roughness * max(maxLod - 1.0, 0.0);
            result.specular = sampleEnvLod(surface.material, dominant, specularLevel);
#endif
        }
    }
#endif

#ifdef LITE3D_ENV_PROBE_MAX // The IBL is enabled in the engine 
    if (hasFlag(surface.material.flags, LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE))
    {
        getProbeMaxLodAndCount(surface.material, probesCount, maxLod);
        float specularLevel = surface.material.roughness * max(maxLod - 1.0, 0.0);

        if (surface.material.environmentSingleProbeIndex >= 0)
        {
            EnvironmentProbeStruct probe = probes[surface.material.environmentSingleProbeIndex];

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
            {
                result.diffuse += irradianceSHL2(surface.normal, probe.sh, visibility);
            }
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
            {
                result.specular += sampleProbeLod(surface.material, dominant, 
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
                    totalSpecular += sampleProbeLod(surface.material, dominant, p, specularLevel) * sW;
                    totalSWeight += sW;
                }
#endif
            }

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            for (uint ir = 0; ir < LITE3D_SH_COEFFS; ++ir)
            {
                radianceSH[ir] /= totalDWeight;
            }

            result.diffuse += irradianceSHL2(surface.normal, radianceSH, visibility);
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            result.specular += totalSpecular / totalSWeight;
#endif
        }

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
        result.diffuse *= M_INV_PI;
#endif
    }
#endif

    vec3 bounceBoost = gtaoMultiBounce(visibility, surface.material.albedo.rgb) / visibility;
    float occlusion = computeSpecularOcclusion(surface, angular, dominant);
    vec3 F = fresnelSchlickRoughness(angular.NdotV, surface);
    vec3 specEnergy = IBLSpecularEnergy(F, angular.NdotV, surface);

    result.diffuseEnergy = diffuseFactor(F, surface.material.metallic);
    result.diffuse *= surface.material.albedo.rgb * result.diffuseEnergy * bounceBoost * surface.material.envDiffuse;
    result.specular *= specEnergy * occlusion * surface.material.envSpecular;

    return result;
}
