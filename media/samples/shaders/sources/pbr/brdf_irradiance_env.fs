#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCube IrradianceProbe;
uniform samplerCube EnvironmentProbe;

vec3 ComputeEnvironmentLighting(vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    // Reflect vector from surface
    vec3 R = reflect(-V, N);
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
    // Duffuse irradiance 
    vec3 globalIrradiance = texture(IrradianceProbe, N).rgb;
    vec3 diffuseEnv = diffuseFactor(F, specular.z) * albedo * globalIrradiance * DIFFUSE_IRRADIANCE_STRENGTH;
    // Specular 
    float specularLevel = clamp(sqrt(specular.y) * float(SPECULAR_MAX_LOD), float(SPECULAR_MIN_LOD), float(SPECULAR_MAX_LOD));
    vec3 specularEnv = textureLod(EnvironmentProbe, R, specularLevel).rgb * F * saFactor;

    return (diffuseEnv + specularEnv) * aoFactor;
}
