#include "samples:shaders/sources/common/utils_inc.glsl"

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float HdotV, vec3 specular)
{
    vec3 F = fresnelSchlickRoughness(HdotV, albedo, specular, FRESNEL_POWER);
    return diffuseFactor(F, specular.z) * albedo / M_PI;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, HdotV, specular) * radiance * NdotL;
}
