#include "samples:shaders/sources/common/utils_inc.glsl"

const vec3 BaseF0 = vec3(0.04);

// Fresnel equation (Schlick)
vec3 FresnelSchlickRoughness(float NdotV, vec3 albedo, vec3 specular)
{
    // Calculate F0 coeff (metalness)
    vec3 F0 = BaseF0;
    F0 = mix(F0, albedo, specular.z);

    vec3 F = F0 + (max(vec3(1.0 - specular.y), F0) - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), FRESNEL_POWER);
    return clamp(F * specular.x, 0.0, 1.0);
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float HdotV, vec3 specular)
{
    vec3 F = FresnelSchlickRoughness(HdotV, albedo, specular);
    // PBR модель строится на принципе сохранения энергии и по этому энергия поглощенного и отраженного 
    // света в суммме не могут быть больше чем энергия падающего луча от источника света  
    // f - Кофф Френеля по сути определяет отраженную часть света, поэтому kD - Кофф поглащенного света
    // вычисляется просто kD = 1 - f , но с поправкой на металл/диэлектрик. Металл хуже поглощает свет.
    vec3 kD = 1.0 - F;
    kD *= 1.0 - specular.z;

    return kD * albedo / M_PI;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, HdotV, specular) * radiance * NdotL;
}
