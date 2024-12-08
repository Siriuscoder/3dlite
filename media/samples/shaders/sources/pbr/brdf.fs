#include "samples:shaders/sources/common/common_inc.glsl"

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(in Surface surface, in AngularInfo angular);
{
    float ndf = NDF(angular.NdotH, surface.material.roughness);
    float g = G(angular.NdotV, angular.NdotL, surface.material.roughness);
    vec3 F = fresnelSchlickRoughness(HdotV, surface.material);

    vec3 s = (ndf * g * F) / (4.0 * angular.NdotV * angular.NdotL);
    return diffuseFactor(F, surface.material.metallic) * surface.material.albedo.rgb / M_PI + s;
}
