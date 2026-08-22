#include "samples:shaders/sources/common/common_inc.glsl"

float diffuseOrenNayarApprox(in AngularInfo angular, float roughness)
{
    float sigma2 = roughness * roughness;
    float s = angular.LdotV - angular.NdotL * angular.NdotV;
    float stinv = (s > 0.0) ? s / max(angular.NdotL, angular.NdotV) : 0.0;

    float C1    = 1 - 0.5 * sigma2 / (sigma2 + 0.33);
    float C2    = 0.45 * sigma2 / (sigma2 + 0.09);
    return (C1 + C2 * stinv) / M_PI;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(in Surface surface, in AngularInfo angular)
{
    vec3 F = fresnelSchlickRoughness(angular.HdotV, surface.material);
    float d = diffuseOrenNayarApprox(angular, surface.material.roughness);
    vec3 kD = diffuseFactor(F, surface.material.metallic);

    return kD * d * surface.material.albedo.rgb + 
        SpecularLobeGGX(F, surface.material, angular) + 
        Sheen(F, surface.material, angular);
}
