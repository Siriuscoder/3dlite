#include "samples:shaders/sources/common/common_inc.glsl"

// Lambertian
SufraceLighting BRDF(in Surface surface, in AngularInfo angular)
{
    vec3 F = fresnelSchlick(angular.HdotV, surface);
    vec3 kD = diffuseFactor(F, surface.material.metallic);

    SufraceLighting sl;
    sl.diffuse = kD * DiffuseLobeLambertian(surface.material);
    sl.specular = vec3(0.0);

    return sl;
}
