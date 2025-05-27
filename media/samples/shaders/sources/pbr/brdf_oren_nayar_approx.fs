#include "samples:shaders/sources/common/common_inc.glsl"

float diffuseOrenNayarApprox(in AngularInfo angular, float roughness)
{
    float s2    = roughness * roughness;
    float VoL   = 2 * angular.HdotV * angular.HdotV - 1;       // double angle identity
    float Cosri = VoL - angular.NdotV * angular.NdotL;
    float C1    = 1 - 0.5 * s2 / (s2 + 0.33);
    float C2    = 0.45 * s2 / (s2 + 0.09) * Cosri * (Cosri >= 0 ? 1.0 / max(angular.NdotL, angular.NdotV + 0.0001) : 1.0);
    return (C1 + C2) * (1 + roughness * 0.5) / M_PI;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(in Surface surface, in AngularInfo angular)
{
    vec3 F = fresnelSchlickRoughness(angular.HdotV, surface.material);
    float d = diffuseOrenNayarApprox(angular, surface.material.roughness);
    vec3 kD = diffuseFactor(F, surface.material.metallic);

    return kD * d * surface.material.albedo.rgb + 
        SpecularGGX(F, surface.material, angular) + 
        Sheen(F, surface.material, angular);
}
