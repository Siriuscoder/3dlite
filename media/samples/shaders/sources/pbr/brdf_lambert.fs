#include "samples:shaders/sources/common/common_inc.glsl"

// Lambertian
vec3 BRDF(in Surface surface, in AngularInfo angular)
{
    vec3 F = fresnelSchlickRoughness(angular.HdotV, surface.material);
    return DiffuseLambertian(F, surface.material);
}
