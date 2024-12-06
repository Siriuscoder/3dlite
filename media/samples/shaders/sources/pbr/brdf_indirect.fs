#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE

#include "samples:shaders/sources/bindless/material_inc.glsl"

int getEnvTextureMaxLod(Surface surface)
{
    return int(round(log2(float(textureSize(surface.material.environment.textureId, 0).x))));
}

ivec2 getProbeTextureMaxLodAndCount(Surface surface)
{
    ivec3 sizes = textureSize(surface.material.environmentProbe.textureId, 0);
    return ivec2(
        int(round(log2(float(sizes.x)))),
        sizes.z
    );
}

vec3 getEnvTextureLod(vec3 uv, float lod)
{
    return textureLod(surface.material.environment.textureId, uv, lod).rgb;
}

vec3 getProbeTextureLod(vec3 uv, int index, float lod)
{
    return textureLod(surface.material.environmentProbe.textureId, vec4(uv, index), lod).rgb;
}

#else

#include "samples:shaders/sources/common/material_inc.glsl"

uniform samplerCube Environment;
uniform samplerCubeArray EnvironmentProbe;
vec3 getEnvTextureMaxLod(Surface surface)
{
    return log2(float(textureSize(Environment, 0).x));
}

ivec2 getProbeTextureMaxLodAndCount(Surface surface)
{
    ivec3 sizes = textureSize(EnvironmentProbe, 0);
    return ivec2(
        int(round(log2(float(sizes.x)))),
        sizes.z
    );
}

vec3 getEnvTextureLod(vec3 uv, float lod)
{
    return textureLod(Environment, uv, lod).rgb;
}

vec3 getProbeTextureLod(vec3 uv, int index, float lod)
{
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
}

#endif

#include "samples:shaders/sources/common/utils_inc.glsl"

#ifndef LITE3D_BASE_AMBIENT_LIGHT
#define LITE3D_BASE_AMBIENT_LIGHT vec3(0.0)
#endif

#ifndef LITE3D_ENV_PROBE_DIFFUSE_POWER
#define LITE3D_ENV_PROBE_DIFFUSE_POWER 2.0
#endif

#ifndef LITE3D_ENV_PROBE_SPECULAR_POWER
#define LITE3D_ENV_PROBE_SPECULAR_POWER 10.0
#endif


layout(std140) uniform EnvProbesData
{
    EnvironmentProbeStruct probes[LITE3D_ENV_PROBE_MAX];
};

//vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float edF, float esF)
vec3 ComputeIndirect(Surface surface, AngularInfo angular)
{
    vec3 diffuseIrradianceLx = vec3(0.0);
    vec3 specularIrradianceLx = vec3(0.0);
    vec3 specular = vec3(surface.material.specular, surface.material.roughness, surface.material.metallic);

    if (hasFlag(surface.material.flags, MATERIAL_ENVIRONMENT_TEXTURE))
    {
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
        if (!hasFlag(surface.material.environment.flags, TEXTURE_FLAG_LOADED))
        {
            return vec3(0.0);
        }
#endif
        int maxLod = getEnvTextureMaxLod(surface);
        // Reflect vector from surface
        vec3 R = reflect(-angular.V, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(angular.NdotV, surface.material.albedo.rgb, specular);
        // Duffuse irradiance 
        diffuseIrradianceLx = diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb * 
            getEnvTextureLod(surface.normal * surface.material.environmentUVScale, maxLod - 1.0);
        // Specular 
        float specularLevel = sqrt(surface.material.roughness) * maxLod;
        specularIrradianceLx = getEnvTextureLod(R * surface.material.environmentUVScale, specularLevel) * F;
    }
    else if (hasFlag(surface.material.flags, MATERIAL_ENVIRONMENT_SINGLE_PROBE))
    {
        ivec2 maxLodAndCount = getProbeTextureMaxLodAndCount(surface);
        float specularLevel = sqrt(specular.y) * maxLodAndCount.x;
        // Reflect vector from surface
        vec3 R = reflect(-angular.V, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
        // Calc indirect light from single probe by index
        if (hasFlag(probes[p].flags, ENV_PROBE_FLAG_IRRADIANCE))
        {
            diffuseIrradianceLx = getProbeTextureLod(surface.normal, surface.material.environmentSingleProbeIndex, maxLod - 1.0);
        }

        if (hasFlag(probes[p].flags, ENV_PROBE_FLAG_SPECULAR))
        {
            specularIrradianceLx = getProbeTextureLod(R, surface.material.environmentSingleProbeIndex, specularLevel);
        }

        diffuseIrradianceLx *= diffuseFactor(F, surface.material.metallic) * albedo;
        specularIrradianceLx *= F;
    }
    else if (hasFlag(surface.material.flags, MATERIAL_ENVIRONMENT_MULTI_PROBE))
    {
        float nearProbeDistance = FLT_MAX;
        ivec2 maxLodAndCount = getProbeTextureMaxLodAndCount(surface);
        int probesCount = maxLodAndCount.y;
        float specularLevel = sqrt(specular.y) * maxLodAndCount.x;
        // Reflect vector from surface
        vec3 R = reflect(-angular.V, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
        // Calc indirect light
        float totalDWeight = FLT_EPSILON;
        float totalSWeight = FLT_EPSILON;
        for (int p = 0; p < probesCount; ++p)
        {
            float probeDistance = length(P - probes[p].position.xyz);
            if (hasFlag(probes[p].flags, ENV_PROBE_FLAG_IRRADIANCE))
            {
                float dW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_DIFFUSE_POWER), FLT_EPSILON);
                diffuseIrradianceLx += getProbeTextureLod(surface.normal, p, maxLod - 1.0) * dW;
                totalDWeight += dW;
            }

            if (hasFlag(probes[p].flags, ENV_PROBE_FLAG_SPECULAR))
            {
                float sW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_SPECULAR_POWER), FLT_EPSILON);
                specularIrradianceLx += getProbeTextureLod(R, p, specularLevel) * sW;
                totalSWeight += sW;
            }
        }

        diffuseIrradianceLx *= diffuseFactor(F, surface.material.metallic) * albedo / totalDWeight;
        specularIrradianceLx *= F / totalSWeight;
    }

    return LITE3D_BASE_AMBIENT_LIGHT + 
        diffuseIrradianceLx * surface.material.envDiffuse + 
        specularIrradianceLx * surface.material.envSpecular;
}
