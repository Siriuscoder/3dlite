#include "samples:shaders/sources/common/material_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 btang;

uniform mat4 projViewMatrix;

out vec2 iuv;
out vec3 iwv;
out vec3 iwn;
out vec3 iwt;
out vec3 iwb;
flat out int drawID;

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID = gl_DrawIDARB;
    
    vec4 wv = invInfo.model * vertex;
    // vertex coordinate in world space 
    iwv = wv.xyz / wv.w;
    // texture coordinate 
    iuv = uv;
    // calculate tangent, normal, binormal in world space
    mat3 normalMatrix = mat3(invInfo.normal[0].xyz, invInfo.normal[1].xyz, invInfo.normal[2].xyz);
    iwt = normalize(normalMatrix * tang);
    iwb = normalize(normalMatrix * btang);
    iwn = normalize(normalMatrix * normal);

    gl_Position = projViewMatrix * wv;
}