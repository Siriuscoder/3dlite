#include "samples:shaders/sources/common/common_inc.glsl"

float diffuseOrenNayar(in AngularInfo angular, float roughness)
{
    float angleVN = acos(angular.NdotV);
    float angleLN = acos(angular.NdotL);
    
    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    float gamma = angular.LdotV - angular.NdotL * angular.NdotV;
    
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
    
    float L1 = angular.NdotL * (C1 + A + B);
    
    // Interreflection
    float twoBetaPi = 2.0 * beta / M_PI;
    float L2 = 0.17 * angular.NdotL * (s2 / (s2 + 0.13)) * (1.0 - gamma * twoBetaPi * twoBetaPi);
    
    return L1 + L2;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(in Surface surface, in AngularInfo angular)
{
    float ndf = NDF(angular.NdotH, surface.material.roughness);
    float g = G(angular.NdotV, angular.NdotL, surface.material.roughness);
    vec3 F = fresnelSchlickRoughness(angular.HdotV, surface.material);

    vec3 s = (ndf * g * F) / (4.0 * angular.NdotV * angular.NdotL);
    float d = diffuseOrenNayar(angular, surface.material.roughness);
    vec3 kD = diffuseFactor(F, surface.material.metallic);

    return kD * d * surface.material.albedo.rgb + s;
}
