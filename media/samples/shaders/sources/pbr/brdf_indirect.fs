#include "samples:shaders/sources/common/common_inc.glsl"

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE

int getEnvTextureMaxLod(in Surface surface)
{
    return int(round(log2(float(textureSize(surface.material.environment.textureId, 0).x))));
}

ivec2 getProbeTextureMaxLodAndCount(in Surface surface)
{
    ivec3 sizes = textureSize(surface.material.environmentProbe.textureId, 0);
    return ivec2(
        int(round(log2(float(sizes.x)))),
        sizes.z
    );
}

vec3 getEnvTextureLod(in Surface surface, vec3 uv, float lod)
{
    return textureLod(surface.material.environment.textureId, uv * surface.material.environmentUVScale, lod).rgb;
}

vec3 getProbeTextureLod(in Surface surface, vec3 uv, uint index, float lod)
{
    return textureLod(surface.material.environmentProbe.textureId, vec4(uv, index), lod).rgb;
}

#else

uniform samplerCube Environment;
uniform samplerCubeArray EnvironmentProbe;

vec3 getEnvTextureMaxLod(in Surface surface)
{
    return log2(float(textureSize(Environment, 0).x));
}

ivec2 getProbeTextureMaxLodAndCount(in Surface surface)
{
    ivec3 sizes = textureSize(EnvironmentProbe, 0);
    return ivec2(
        int(round(log2(float(sizes.x)))),
        sizes.z
    );
}

vec3 getEnvTextureLod(in Surface surface, vec3 uv, float lod)
{
    return textureLod(Environment, uv * surface.material.environmentUVScale, lod).rgb;
}

vec3 getProbeTextureLod(in Surface surface, vec3 uv, uint index, float lod)
{
    return textureLod(EnvironmentProbe, vec4(uv, index), lod).rgb;
}

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

layout(std140) uniform EnvProbesData
{
    EnvironmentProbeStruct probes[LITE3D_ENV_PROBE_MAX];
};

vec3 ComputeIndirect(in Surface surface, in AngularInfo angular)
{
    vec3 diffuseIrradianceLx = vec3(0.0);
    vec3 specularIrradianceLx = vec3(0.0);

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
        vec3 R = reflect(-angular.viewDir, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(angular.NdotV, surface.material);
        // Duffuse irradiance 
        diffuseIrradianceLx = diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb * 
            getEnvTextureLod(surface, surface.normal, maxLod - 1.0);
        // Specular 
        float specularLevel = sqrt(surface.material.roughness) * maxLod;
        specularIrradianceLx = getEnvTextureLod(surface, R, specularLevel) * F;
    }
    else if (hasFlag(surface.material.flags, MATERIAL_ENVIRONMENT_SINGLE_PROBE))
    {
        ivec2 maxLodAndCount = getProbeTextureMaxLodAndCount(surface);
        float specularLevel = sqrt(surface.material.roughness) * maxLodAndCount.x;
        // Reflect vector from surface
        vec3 R = reflect(-angular.viewDir, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(angular.NdotV, surface.material);

        EnvironmentProbeStruct probe = probes[surface.material.environmentSingleProbeIndex];
        // Calc indirect light from single probe by index
        if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
        {
            diffuseIrradianceLx = getProbeTextureLod(surface, surface.normal, surface.material.environmentSingleProbeIndex, maxLodAndCount.x - 1.0);
        }

        if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
        {
            specularIrradianceLx = getProbeTextureLod(surface, R, surface.material.environmentSingleProbeIndex, specularLevel);
        }

        diffuseIrradianceLx *= diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb;
        specularIrradianceLx *= F;
    }
    else if (hasFlag(surface.material.flags, MATERIAL_ENVIRONMENT_MULTI_PROBE))
    {
        float nearProbeDistance = FLT_MAX;
        ivec2 maxLodAndCount = getProbeTextureMaxLodAndCount(surface);
        uint probesCount = maxLodAndCount.y;
        float specularLevel = sqrt(surface.material.roughness) * maxLodAndCount.x;
        // Reflect vector from surface
        vec3 R = reflect(-angular.viewDir, surface.normal);
        // Fresnel by Schlick aproxx
        vec3 F = fresnelSchlickRoughness(angular.NdotV, surface.material);

        for (uint p = 0; p < probesCount; ++p)
        {
            float probeDistance = length(surface.wv - probes[p].position.xyz);
            nearProbeDistance = min(nearProbeDistance, probeDistance);
        }

        // Calc indirect light
        float totalDWeight = FLT_EPSILON;
        float totalSWeight = FLT_EPSILON;
        for (uint p = 0; p < probesCount; ++p)
        {
            EnvironmentProbeStruct probe = probes[p];
            float probeDistance = length(surface.wv - probe.position.xyz);
            float relativeDistance = nearProbeDistance / max(probeDistance, FLT_EPSILON);

            if (relativeDistance < LITE3D_ENV_PROBE_RELATIVE_DISTANCE_THRESHOLD)
                continue;
            
            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_IRRADIANCE))
            {
                float dW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_DIFFUSE_POWER), FLT_EPSILON);
                diffuseIrradianceLx += getProbeTextureLod(surface, surface.normal, p, maxLodAndCount.x - 1.0) * dW;
                totalDWeight += dW;
            }

            if (hasFlag(probe.flags, LITE3D_ENV_PROBE_FLAG_SPECULAR))
            {
                float sW = 1.0 / max(pow(probeDistance, LITE3D_ENV_PROBE_SPECULAR_POWER), FLT_EPSILON);
                specularIrradianceLx += getProbeTextureLod(surface, R, p, specularLevel) * sW;
                totalSWeight += sW;
            }
        }

        diffuseIrradianceLx *= diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb / totalDWeight;
        specularIrradianceLx *= F / totalSWeight;
    }

    return diffuseIrradianceLx * surface.material.envDiffuse + 
        specularIrradianceLx * surface.material.envSpecular;
}
