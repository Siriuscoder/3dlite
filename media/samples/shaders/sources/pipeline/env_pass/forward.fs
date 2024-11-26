#ifdef BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, vec3(0.0), vec3(0.0));

    vec3 total = ComputeIllumination(surface.wv, 
        surface.normal, 
        surface.material.albedo.rgb, 
        surface.material.emission.rgb, 
        vec3(surface.material.specular, surface.material.roughness, surface.material.metallic), 
        surface.ao, 
        surface.material.envSpecular);
    // Final
    fragColor = vec4(total, surface.material.alpha);
}
