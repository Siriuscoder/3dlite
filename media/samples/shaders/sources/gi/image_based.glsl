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
#elif defined(LITE3D_ENABLE_ENVIRONMENT_TEXTURE)
    return log2(float(textureSize(Environment, 0).x));
#else
    return 1.0;
#endif
}

vec3 sampleEnvLod(in Material material, vec3 uv, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(samplerCube(material.environment.textureId), uv * material.environmentUVScale, lod).rgb;
#elif defined(LITE3D_ENABLE_ENVIRONMENT_TEXTURE)
    return textureLod(Environment, uv * material.environmentUVScale, lod).rgb;
#else
    return vec3(0.0);
#endif
}

void getProbeMaxLodAndCount(in Material material, inout int count, inout float maxLod)
{
    ivec3 sizes;
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    sizes = textureSize(samplerCubeArray(material.environmentProbe.textureId), 0);
#elif defined(LITE3D_ENV_PROBE_MAX)
    sizes = textureSize(EnvironmentProbe, 0);
#endif

    count = sizes.z;
    maxLod = log2(float(sizes.x));
}

vec3 sampleProbeLod(in Material material, vec3 uv, int index, float lod)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(samplerCubeArray(material.environmentProbe.textureId), vec4(uv, index), lod).rgb;
#elif defined(LITE3D_ENV_PROBE_MAX)
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
#else 
    return vec3(0.0);
#endif
}

float sampleProbeDepth(in Material material, vec3 uv, int index)
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    return textureLod(samplerCubeArray(material.environmentProbeDepth.textureId), vec4(uv, index), 0).r;
#elif defined(LITE3D_ENV_PROBE_MAX)
    return textureLod(EnvironmentProbeDepth, vec4(uv, index), 0).r;
#else 
    return 0.0;
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

float distanceWeight(float dist, float nearestDistance)
{
    float scale = max(LITE3D_ENV_PROBE_MIN_SPACING, nearestDistance * 0.75);
    float delta = max(dist - nearestDistance, 0.0);

    float weight = exp(-delta / scale);

    float fade = 1.0 - smoothstep(2.0 * scale, 3.0 * scale, delta);
    return weight * fade;
}

float specularDistanceWeight(in Material material, float dist, float nearestDistance)
{
    float sharpness = mix(
        12.0,  // smooth surfaces
        3.0,   // rough surfaces
        material.roughness);

    float ratio = (nearestDistance + 0.1) /
                  (dist + 0.1);

    return pow(ratio, sharpness);
}

float normalWeight(vec3 N, vec3 toProbe)
{
    float NdotL = dot(N, toProbe);
    float weight = smoothstep(-0.35, 0.5, NdotL);

    return mix(0.1, 1.0, weight);
}

float visibilityWeight(in Material material, vec3 fragPos, vec3 fromProbeToFrag, vec3 fromProbeToFragDir, int probeIndex)
{
    // Глубина вдоль оптической оси cubemap-грани
    float fragmentZ = max(
        max(abs(fromProbeToFrag.x), abs(fromProbeToFrag.y)),
        abs(fromProbeToFrag.z)
    );

    if (fragmentZ < LITE3D_ENV_PROBE_ZNEAR || fragmentZ > LITE3D_ENV_PROBE_ZFAR)
        return 0.0;

    // Sample depth cubemap
    float storedDepth = sampleProbeDepth(material, fromProbeToFragDir, probeIndex);
    // Both depths represent distances along the face forward axis
    float storedZ = linearizeDepth(storedDepth, LITE3D_ENV_PROBE_ZNEAR, LITE3D_ENV_PROBE_ZFAR);

    //float delta = fragmentZ - storedZ;
//
    float bias     = 2.2;
    float softness = max(0.5, fragmentZ * 0.15);
//
    //return 1.0 - smoothstep(bias, bias + transition, delta);

    float delta = max(fragmentZ - storedZ - bias, 0.0);
    return exp(-delta / max(softness, 0.001));
}

SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular)
{
    SufraceLighting result = SufraceLighting(vec3(0.0), vec3(0.0), vec3(0.0));
    float maxLod = 0.0;
    int probesCount = 0;
    const float visibility = surface.ao;

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
            result.diffuse = sampleEnvLod(surface.material, surface.bentNormal, maxLod - 1.0);
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
                result.diffuse += irradianceSHL2(surface.bentNormal, probe.sh, visibility);
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
            float nearestProbeDistance = FLT_MAX;
            // Calc indirect light
            float totalDWeight = FLT_EPSILON;
            float totalSWeight = FLT_EPSILON;
            vec3 totalSpecular = vec3(0.0);

            vec4 radianceSH[LITE3D_SH_COEFFS] = {
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0), 
                vec4(0.0)
            };

            for (int p = 0; p < probesCount; ++p)
            {
                EnvironmentProbeStruct probe = probes[p];
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_REUSABLE)) // Empty probe, may be reused
                    continue;

                float probeDistance = distance(surface.wv, probe.position.xyz);
                nearestProbeDistance = min(nearestProbeDistance, probeDistance);
            }

            for (int p = 0; p < probesCount; ++p)
            {
                EnvironmentProbeStruct probe = probes[p];
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_REUSABLE)) // Empty probe, may be reused
                    continue;

                vec3 fromProbeToFrag = surface.wv - probe.position.xyz;
                float probeDistance = length(fromProbeToFrag);

                float dW = distanceWeight(probeDistance, nearestProbeDistance);
                if (dW < LITE3D_ENV_PROBE_MIN_WEIGHT)
                    continue;

                vec3 fromProbeToFragDir = normalize(fromProbeToFrag);
                float nW = normalWeight(surface.normal, -fromProbeToFragDir);
                float vW = visibilityWeight(surface.material, surface.wv, fromProbeToFrag, fromProbeToFragDir, p);

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
                {
                    float tW = dW * nW * vW;
                    
                    if (tW > LITE3D_ENV_PROBE_MIN_WEIGHT)
                    {
                    // Interpolate SH coeffs between probes 
                    for (uint ir = 0; ir < LITE3D_SH_COEFFS; ++ir)
                    {
                            radianceSH[ir] += probe.sh[ir] * tW;
                        }

                        totalDWeight += tW;
                    }
                }
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
                if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
                {
                    float sW = specularDistanceWeight(surface.material, probeDistance, nearestProbeDistance);
                    float tW = dW * nW * sW;

                    if (tW > LITE3D_ENV_PROBE_MIN_WEIGHT)
                    {
                        totalSpecular += sampleProbeLod(surface.material, dominant, p, specularLevel) * tW;
                        totalSWeight += tW;
                    }
                }
#endif
            }

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
            if (totalDWeight > LITE3D_ENV_PROBE_MIN_WEIGHT)
            {
                for (uint ir = 0; ir < LITE3D_SH_COEFFS; ++ir)
            {
                radianceSH[ir] /= totalDWeight;
            }

            result.diffuse += irradianceSHL2(surface.bentNormal, radianceSH, visibility);
            }
#endif

#ifdef LITE3D_EVALUATE_IBL_SPECULAR
            if (totalDWeight > LITE3D_ENV_PROBE_MIN_WEIGHT)
            {
            result.specular += totalSpecular / totalSWeight;
            }
#endif
        }

#ifdef LITE3D_EVALUATE_IBL_DIFFUSE
        result.diffuse *= M_INV_PI;
#endif
    }
#endif

    vec3 bounceBoost = gtaoMultiBounce(visibility, surface.material.albedo.rgb);
    float occlusion = computeSpecularOcclusion(surface, angular, dominant);
    vec3 F = fresnelSchlickRoughness(angular.NdotV, surface);
    vec3 specEnergy = IBLSpecularEnergy(F, angular.NdotV, surface);

    result.diffuseEnergy = diffuseFactor(F, surface.material.metallic);
    result.diffuse *= surface.material.albedo.rgb * result.diffuseEnergy * bounceBoost * surface.material.envDiffuse;
    result.specular *= specEnergy * occlusion * surface.material.envSpecular;

    return result;
}
