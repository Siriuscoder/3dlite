#include "samples:shaders/sources/common/common_inc.glsl"

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(in Surface surface, in AngularInfo angular);
{
    vec3 F = fresnelSchlickRoughness(angular.HdotV, surface.material);
    return DiffuseLambertian(F, surface.material) + 
        SpecularGGX(F, surface.material, angular) + 
        Sheen(F, surface.material, angular);
}
