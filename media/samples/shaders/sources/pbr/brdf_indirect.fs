#include "samples:shaders/sources/common/common_inc.glsl"

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE

float getEnvTextureMaxLod(in Material material)
{
    return log2(float(textureSize(material.environment.textureId, 0).x));
}

vec3 getEnvTextureLod(in Material material, vec3 uv, float lod)
{
    return textureLod(material.environment.textureId, uv * material.environmentUVScale, lod).rgb;
}

void getProbeTextureMaxLodAndCount(in Material material, inout uint count, inout float maxLod)
{
    ivec3 sizes = textureSize(material.environmentProbe.textureId, 0);
    count = uint(sizes.z);
    maxLod = log2(float(sizes.x));
}

vec3 getProbeTextureLod(in Material material, vec3 uv, uint index, float lod)
{
    return textureLod(material.environmentProbe.textureId, vec4(uv, index), lod).rgb;
}

#else

uniform samplerCube Environment;

float getEnvTextureMaxLod(in Material material)
{
    return log2(float(textureSize(Environment, 0).x));
}

vec3 getEnvTextureLod(in Material material, vec3 uv, float lod)
{
    return textureLod(Environment, uv * material.environmentUVScale, lod).rgb;
}

#ifdef LITE3D_ENV_PROBE_MAX

uniform samplerCubeArray EnvironmentProbe;

vec3 getProbeTextureLod(in Material material, vec3 uv, uint index, float lod)
{
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
}

void getProbeTextureMaxLodAndCount(in Material material, inout uint count, inout float maxLod)
{
    ivec3 sizes = textureSize(EnvironmentProbe, 0);
    count = uint(sizes.z);
    maxLod = log2(float(sizes.x));
}

#endif
#endif

#ifndef LITE3D_ENV_PROBE_DIFFUSE_POWER
#define LITE3D_ENV_PROBE_DIFFUSE_POWER 2.0
#endif

#ifndef LITE3D_ENV_PROBE_SPECULAR_POWER
#define LITE3D_ENV_PROBE_SPECULAR_POWER 10.0
#endif

#ifndef LITE3D_ENV_PROBE_RELATIVE_DISTANCE_THRESHOLD
#define LITE3D_ENV_PROBE_RELATIVE_DISTANCE_THRESHOLD 0.04
#endif

#ifdef LITE3D_ENV_PROBE_MAX
layout(std140) uniform EnvProbesData
{
    EnvironmentProbeStruct probes[LITE3D_ENV_PROBE_MAX];
};
#endif

vec3 ComputeIndirect(in Surface surface, in AngularInfo angular)
{
    vec3 diffuseIrradianceLx = vec3(0.0f);
    vec3 specularIrradianceLx = vec3(0.0f);
    float maxLod = 0.0f;
    uint probesCount = 0;

    // Reflect vector from surface
    vec3 R = reflect(-angular.viewDir, surface.normal);
    // Fresnel by Schlick aprox
    vec3 F = fresnelSchlickRoughness(angular.NdotV, surface.material);

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
            maxLod = getEnvTextureMaxLod(surface.material);
            // Duffuse irradiance 
            diffuseIrradianceLx += getEnvTextureLod(surface.material, surface.normal, maxLod - 1.0);
            // Specular 
            float specularLevel = sqrt(surface.material.roughness) * maxLod;
            specularIrradianceLx += getEnvTextureLod(surface.material, R, specularLevel);
        }
    }
#endif

#ifdef LITE3D_ENV_PROBE_MAX // The IBL is enabled in the engine 
    if (hasFlag(surface.material.flags, LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE))
    {
        float nearProbeDistance = FLT_MAX;
        getProbeTextureMaxLodAndCount(surface.material, probesCount, maxLod);
        float specularLevel = sqrt(surface.material.roughness) * maxLod;

        for (uint p = 0u; p < probesCount; ++p)
        {
            float probeDistance = length(surface.wv - probes[p].position.xyz);
            nearProbeDistance = min(nearProbeDistance, probeDistance);
        }

        // Calc indirect light
        float totalDWeight = FLT_EPSILON;
        float totalSWeight = FLT_EPSILON;
        vec3 totalDiffuse = vec3(0.0f);
        vec3 totalSpecular = vec3(0.0f);
        for (uint p = 0u; p < probesCount; ++p)
        {
            EnvironmentProbeStruct probe = probes[p];
            float probeDistance = length(surface.wv - probe.position.xyz);
            float relativeDistance = nearProbeDistance / max(probeDistance, FLT_EPSILON);

            if (relativeDistance < LITE3D_ENV_PROBE_RELATIVE_DISTANCE_THRESHOLD)
                continue;
            
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
            {
                float dW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_DIFFUSE_POWER), FLT_EPSILON);
                totalDiffuse += getProbeTextureLod(surface.material, surface.normal, p, maxLod - 1.0) * dW;
                totalDWeight += dW;
            }

            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
            {
                float sW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_SPECULAR_POWER), FLT_EPSILON);
                totalSpecular += getProbeTextureLod(surface.material, R, p, specularLevel) * sW;
                totalSWeight += sW;
            }
        }

        diffuseIrradianceLx += totalDiffuse / totalDWeight;
        specularIrradianceLx += totalSpecular / totalSWeight;
    }
    else if (hasFlag(surface.material.flags, LITE3D_MATERIAL_ENVIRONMENT_SINGLE_PROBE))
    {
        getProbeTextureMaxLodAndCount(surface.material, probesCount, maxLod);
        float specularLevel = sqrt(surface.material.roughness) * maxLod;

        EnvironmentProbeStruct probe = probes[surface.material.environmentSingleProbeIndex];
        // Calc indirect light from single probe by index
        if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
        {
            diffuseIrradianceLx += getProbeTextureLod(surface.material, surface.normal, surface.material.environmentSingleProbeIndex, maxLod - 1.0);
        }

        if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
        {
            specularIrradianceLx += getProbeTextureLod(surface.material, R, surface.material.environmentSingleProbeIndex, specularLevel);
        }
    }
#endif

    diffuseIrradianceLx *= diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb * surface.material.envDiffuse;
    specularIrradianceLx *= F * surface.material.envSpecular;

    return (diffuseIrradianceLx + specularIrradianceLx) * surface.ao;
}
