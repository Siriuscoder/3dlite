#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

out vec2 iuv_g;
flat out int drawID_g;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID_g = gl_DrawIDARB;

    iuv_g = uv;
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
}