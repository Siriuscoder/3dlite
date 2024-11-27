#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

layout(location = 0) out vec4 channel01;
layout(location = 1) out vec4 channel02;
layout(location = 2) out vec4 channel03;
layout(location = 3) out vec4 channel04;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);
    surfaceAlphaClip(surface); // Alpha clip case 

    channel01 = vec4(iwv, surface.index);
    channel02 = vec4(surface.normal, surface.material.emission.r);
    channel03 = vec4(surface.material.albedo.rgb, surface.material.emission.g);
    channel04 = vec4(surface.material.specular, surface.material.roughness, surface.material.metallic, surface.material.emission.b);
}