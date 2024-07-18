#include "samples:shaders/sources/common/utils_inc.glsl"

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float NdotV, float NdotH, vec3 specular)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    vec3 F = fresnelSchlickRoughness(HdotV, albedo, specular);

    vec3 s = (ndf * g * F) / (4.0 * NdotV * NdotL);
    return diffuseFactor(F, specular.z) * albedo / M_PI + s;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    float NdotH = max(dot(N, H), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, NdotL, HdotV, NdotV, NdotH, specular) * radiance * NdotL;
}
