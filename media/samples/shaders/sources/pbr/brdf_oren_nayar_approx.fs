#include "samples:shaders/sources/common/utils_inc.glsl"

float diffuseOrenNayarApprox(float NdotL, float HdotV, float NdotV, float roughness)
{
    float s2    = roughness * roughness;
    float VoL   = 2 * HdotV * HdotV - 1;       // double angle identity
    float Cosri = VoL - NdotV * NdotL;
    float C1    = 1 - 0.5 * s2 / (s2 + 0.33);
    float C2    = 0.45 * s2 / (s2 + 0.09) * Cosri * (Cosri >= 0 ? 1.0 / max(NdotL, NdotV + 0.0001) : 1.0);
    return (C1 + C2) * (1 + roughness * 0.5) / M_PI;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float NdotV, float NdotH, vec3 specular)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    vec3 F = fresnelSchlickRoughness(HdotV, albedo, specular);

    vec3 s = (ndf * g * F) / (4.0 * NdotV * NdotL);
    float d = diffuseOrenNayarApprox(NdotL, HdotV, NdotV, specular.y);
    vec3 kD = diffuseFactor(F, specular.z);

    return kD * d * albedo + s;
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
