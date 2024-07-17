#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCube Environment;

vec3 ComputeEnvironmentLighting(vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    // Reflect vector from surface
    vec3 R = reflect(-V, N);
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
    // Duffuse irradiance 
    vec3 globalIrradiance = textureLod(Environment, N * CUBE_MAP_UV_SCALE, IRRADIANCE_DIFFUSE_LOD).rgb;
    vec3 diffuseEnv = diffuseFactor(F, specular.z) * globalIrradiance * DIFFUSE_IRRADIANCE_STRENGTH;
    // Specular 
    float specularLevel = clamp(sqrt(specular.y) * SPECULAR_MAX_LOD, SPECULAR_MIN_LOD, SPECULAR_MAX_LOD);
    vec3 specularEnv = textureLod(Environment, R * CUBE_MAP_UV_SCALE, specularLevel).rgb * F * saFactor;

    return (diffuseEnv + specularEnv) * albedo * aoFactor;
}
