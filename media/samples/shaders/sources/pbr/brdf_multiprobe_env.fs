#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCubeArray EnvironmentProbe;

struct EnvironmentProbeStruct
{
    vec4 position;
    mat4 projView[6];
};

layout(std140) uniform EnvProbes
{
    EnvironmentProbeStruct probes[ENV_PROBES_MAX];
};

vec3 ComputeEnvironmentLighting(vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    vec3 diffuseIrradianceLx = vec3(0.0);
    vec3 specularIrradianceLx = vec3(0.0);
    float nearProbeDistance = FLT_MAX;
    ivec3 environmentProbeDimensions = textureSize(EnvironmentProbe, 0);
    float maxLod = log2(float(environmentProbeDimensions.x));
    int probesCount = environmentProbeDimensions.z;
    float specularLevel = sqrt(specular.y) * maxLod;

    // Reflect vector from surface
    vec3 R = reflect(-V, N);
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);

    for (int p = 0; p < probesCount; ++p)
    {
        float probeDistance = length(P - probes[p].position.xyz);
        nearProbeDistance = min(nearProbeDistance, probeDistance);
    }

    // Calc indirect diffuse light
    for (int p = 0; p < probesCount; ++p)
    {
        float probeDistance = length(P - probes[p].position.xyz);
        float weight = nearProbeDistance / probeDistance;

        if (weight < DIFFUSE_IRRADIANCE_WEIGHT_THRESHOLD)
            continue;

        diffuseIrradianceLx += textureLod(EnvironmentProbe, vec4(N, p), maxLod - 1.0).rgb * shlickPow(weight, 2.5);
        specularIrradianceLx += textureLod(EnvironmentProbe, vec4(R, p), specularLevel).rgb * shlickPow(weight, 16.0);
    }

    vec3 kD = diffuseFactor(F, specular.z) * albedo;
    vec3 kS = F;

    diffuseIrradianceLx *= kD;
    specularIrradianceLx *= kS;

    return (diffuseIrradianceLx + specularIrradianceLx) * aoFactor * saFactor;
}
