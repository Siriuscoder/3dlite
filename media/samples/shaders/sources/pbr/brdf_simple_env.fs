#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCube Environment;

vec3 ComputeEnvironmentLighting(vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    float maxLod = log2(float(textureSize(Environment, 0).x));
    // Reflect vector from surface
    vec3 R = reflect(-V, N);
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
    // Duffuse irradiance 
    vec3 globalIrradiance = textureLod(Environment, N * CUBE_MAP_UV_SCALE, maxLod - 1.0).rgb;
    vec3 diffuseEnv = diffuseFactor(F, specular.z) * albedo * globalIrradiance * DIFFUSE_IRRADIANCE_STRENGTH;
    // Specular 
    float specularLevel = sqrt(specular.y) * maxLod;
    vec3 specularEnv = textureLod(Environment, R * CUBE_MAP_UV_SCALE, specularLevel).rgb * F * saFactor;

    return (diffuseEnv + specularEnv) * aoFactor;
}
