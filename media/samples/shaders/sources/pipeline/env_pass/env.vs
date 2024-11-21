#extension GL_ARB_bindless_texture : enable 
#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec2 iuv_g;
out vec3 iwn_g;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    // texture coordinate 
    iuv_g = uv;
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
    // calculate normal in world space
    iwn_g = normalize(invInfo.normal * normal);
}