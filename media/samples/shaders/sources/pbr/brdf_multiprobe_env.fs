#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCubeArray EnvironmentProbe;

layout(std140) uniform EnvProbesData
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
    float totalDWeight = 0.0;
    float totalSWeight = 0.0;
    for (int p = 0; p < probesCount; ++p)
    {
        float probeDistance = length(P - probes[p].position.xyz);
        float dW = 1.0 / max(probeDistance * probeDistance, FLT_EPSILON);
        float relativeDistance = nearProbeDistance / max(probeDistance, FLT_EPSILON);

        if (relativeDistance < DIFFUSE_IRRADIANCE_WEIGHT_THRESHOLD)
            continue;

        float sW = shlickPow(relativeDistance, 16.0);
        diffuseIrradianceLx += textureLod(EnvironmentProbe, vec4(N, p), maxLod - 1.0).rgb * dW;
        specularIrradianceLx += textureLod(EnvironmentProbe, vec4(R, p), specularLevel).rgb * sW;

        totalDWeight += dW;
        totalSWeight += sW;
    }

    vec3 kD = diffuseFactor(F, specular.z) * albedo;
    vec3 kS = F;

    diffuseIrradianceLx *= kD / totalDWeight;
    specularIrradianceLx *= kS / totalSWeight;

    return (diffuseIrradianceLx + specularIrradianceLx) * aoFactor * saFactor;
}
