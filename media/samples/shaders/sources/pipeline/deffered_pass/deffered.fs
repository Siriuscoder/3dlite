#include "samples:shaders/sources/common/material_inc.glsl"

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

    channel01 = vec4(iwv, float(chunkInfo.materialIdx));
    channel02 = vec4(surface.normal, surface.material.Emission.r);
    channel03 = vec4(surface.material.Albedo, surface.material.Emission.g);
    channel04 = vec4(surface.material.Specular, surface.material.Roughness, surface.material.Metallic, surface.material.Emission.b);
}