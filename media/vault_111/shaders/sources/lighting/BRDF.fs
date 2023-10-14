#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

// Fresnel equation (Schlick)
vec3 FresnelSchlickRoughness(float NdotV, vec3 albedo, vec3 specular)
{
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, specular.z);

    vec3 F = F0 + (max(vec3(1.0 - specular.y), F0) - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
    return F * specular.x;
}

// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
	
    return num / denom;
}

// Geometry function (Schlick-Beckmann, Schlick-GGX)
float GGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

// Geometry function (Smith's)
float G(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GGX(NdotV, roughness);
    float ggx1  = GGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float NdotV, float NdotH, vec3 specular, vec3 F)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    // PBR модель строится на принципе сохранения энергии и по этому энергия поглощенного и отраженного 
    // света в суммме не могут быть больше чем энергия падающего луча от источника света  
    // f - Кофф Френеля по сути определяет отраженную часть света, поэтому kD - Кофф поглащенного света
    // вычисляется просто kD = 1 - f , но с поправкой на металл/диэлектрик. Металл хуже поглощает свет.
    vec3 kD = 1.0 - F;
    kD *= 1.0 - specular.z;
        
    vec3 s = (ndf * g * F) / (4.0 * NdotV * NdotL);
    return kD * albedo / M_PI + s;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, vec3 F, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    float NdotH = max(dot(N, H), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, NdotL, HdotV, NdotV, NdotH, specular, F) * radiance * NdotL;
}
