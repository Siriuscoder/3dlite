#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec2 iuv_g;
out vec3 iwn_g;
flat out int drawID_g;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID_g = gl_DrawIDARB;
    // texture coordinate 
    iuv_g = uv;
    // vertex coordinate in world space 
    mat3 normalMatrix = mat3(invInfo.normal[0].xyz, invInfo.normal[1].xyz, invInfo.normal[2].xyz);
    gl_Position = invInfo.model * vertex;
    // calculate normal in world space
    iwn_g = normalize(normalMatrix * normal);
}