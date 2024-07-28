#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCubeArray EnvironmentProbe;

vec3 ComputeEnvironmentLighting(vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    // Reflect vector from surface
    vec3 R = reflect(-V, N);
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
    // Duffuse irradiance 
 //  vec3 globalIrradiance = texture(IrradianceProbe, N * CUBE_MAP_UV_SCALE).rgb;
///vec3 diffuseEnv = diffuseFactor(F, specular.z) * albedo * globalIrradiance * DIFFUSE_IRRADIANCE_STRENGTH;
    // Specular 
   // float specularLevel = clamp(sqrt(specular.y) * float(SPECULAR_MAX_LOD), float(SPECULAR_MIN_LOD), float(SPECULAR_MAX_LOD));
 //   vec3 specularEnv = textureLod(EnvironmentProbe, R * CUBE_MAP_UV_SCALE, specularLevel).rgb * F * saFactor;

    vec3 c = texture(EnvironmentProbe, vec4(R, 1)).rgb;
    return c;
}
