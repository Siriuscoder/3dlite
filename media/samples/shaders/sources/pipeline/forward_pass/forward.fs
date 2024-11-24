#include "samples:shaders/sources/common/material_inc.glsl"

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);

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
