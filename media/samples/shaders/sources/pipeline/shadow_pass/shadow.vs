#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

out vec2 vsUV;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();

    vsUV = uv;
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
}