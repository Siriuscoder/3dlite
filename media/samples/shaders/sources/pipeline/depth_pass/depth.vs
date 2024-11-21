#extension GL_ARB_bindless_texture : enable 
#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 projViewMatrix;

out vec2 iuv;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    vec4 wv = invInfo.model * vertex;
    iuv = uv;

    gl_Position = projViewMatrix * wv;
}