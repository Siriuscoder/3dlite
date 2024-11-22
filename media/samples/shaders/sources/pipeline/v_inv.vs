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

void main()
{
    ChunkInvocationInfo invInfo = getInvocationInfo();
    vec4 wv = invInfo.model * vertex;
    // vertex coordinate in world space 
    iwv = wv.xyz / wv.w;
    // texture coordinate 
    iuv = uv;
    // calculate tangent, normal, binormal in world space
    iwt = normalize(invInfo.normal * tang);
    iwb = normalize(invInfo.normal * btang);
    iwn = normalize(invInfo.normal * normal);

    gl_Position = projViewMatrix * wv;
}