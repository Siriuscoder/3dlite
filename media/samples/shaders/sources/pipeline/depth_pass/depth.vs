#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 projViewMatrix;

out vec2 iuv;
flat out int drawID;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID = gl_DrawIDARB;
    iuv = uv;

    vec4 wv = invInfo.model * vertex;
    gl_Position = projViewMatrix * wv;
}