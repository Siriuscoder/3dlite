#ifdef BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

in vec2 iuv;
out vec4 fragColor;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
    Surface surface = restoreSurface(iuv);
    // Compute total illumination 
    vec3 total = ComputeIllumination(surface.wv, 
        surface.normal, 
        surface.material.albedo.rgb, 
        surface.material.emission.rgb, 
        vec3(surface.material.specular, surface.material.roughness, surface.material.metallic), 
        surface.ao, 
        surface.material.envSpecular);

    fragColor = vec4(total, 1.0);
}
