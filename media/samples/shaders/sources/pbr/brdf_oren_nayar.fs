#include "samples:shaders/sources/common/utils_inc.glsl"

float diffuseOrenNayar(float NdotL, float NdotV, float LdotV, float roughness)
{
    float angleVN = acos(NdotV);
    float angleLN = acos(NdotL);
    
    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    float gamma = LdotV - NdotL * NdotV;
    
    float s2 = roughness * roughness;
    float ss9 = (s2 / (s2 + 0.09));
    
    // C1, C2, and C3
    float C1 = 1.0 - 0.5 * (s2 / (s2 + 0.33));
    float C2 = 0.45 * ss9;
    
    if (gamma >= 0.0) 
    {
        C2 *= sin(alpha);
    } 
    else 
    {
        C2 *= (sin(alpha) - pow((2.0 * beta) / M_PI, 3.0));
    }
    
    float powValue = (4.0 * alpha * beta) / (M_PI * M_PI);
    float C3 = 0.125 * ss9 * powValue * powValue;
    
    // Avoid asymptote at pi/2
    float asym = M_PI / 2.0;
    float lim1 = asym + 0.01;
    float lim2 = asym - 0.01;

    float ab2 = (alpha + beta) / 2.0;

    if (beta >= asym && beta < lim1)
        beta = lim1;
    else if (beta < asym && beta >= lim2)
        beta = lim2;

    if (ab2 >= asym && ab2 < lim1)
        ab2 = lim1;
    else if (ab2 < asym && ab2 >= lim2)
        ab2 = lim2;
    
    // Reflection
    float A = gamma * C2 * tan(beta);
    float B = (1.0 - abs(gamma)) * C3 * tan(ab2);
    
    float L1 = NdotL * (C1 + A + B);
    
    // Interreflection
    float twoBetaPi = 2.0 * beta / M_PI;
    float L2 = 0.17 * NdotL * (s2 / (s2 + 0.13)) * (1.0 - gamma * twoBetaPi * twoBetaPi);
    
    return L1 + L2;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float LdotV, float NdotV, float NdotH, vec3 specular)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    vec3 F = fresnelSchlickRoughness(HdotV, albedo, specular);

    vec3 s = (ndf * g * F) / (4.0 * NdotV * NdotL);
    float d = diffuseOrenNayar(NdotL, NdotV, LdotV, specular.y);
    vec3 kD = diffuseFactor(F, specular.z);

    return kD * d * albedo + s;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    float LdotV = max(dot(L, V), FLT_EPSILON);
    float NdotH = max(dot(N, H), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, NdotL, HdotV, LdotV, NdotV, NdotH, specular) * radiance * NdotL;
}
