#include "samples:shaders/sources/common/common_inc.glsl"

// cook-torrance bidirectional reflective distribution function
SufraceLighting BRDF(in Surface surface, in AngularInfo angular);
{
    vec3 F = fresnelSchlick(angular.HdotV, surface);
    vec3 kD = diffuseFactor(F, surface.material.metallic);

    SufraceLighting sl;
    sl.diffuseEnergy = kD;
    sl.diffuse = DiffuseLobeLambertian(surface.material);
    sl.specular = SpecularLobeGGX(F, surface.material, angular);

    return sl;
}
