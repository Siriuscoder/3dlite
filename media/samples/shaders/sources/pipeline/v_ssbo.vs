#include "samples:shaders/sources/common/utils_pbr_inc.glsl"

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
    ChunkInvocationInfo chunkInfo = chunksInvocationInfo[gl_DrawID];
    vec4 wv = chunkInfo.model * vertex;
    // vertex coordinate in world space 
    iwv = wv.xyz / wv.w;
    // texture coordinate 
    iuv = uv;
    // calculate tangent, normal, binormal in world space
    iwt = normalize(chunkInfo.normal * tang);
    iwb = normalize(chunkInfo.normal * btang);
    iwn = normalize(chunkInfo.normal * normal);

    gl_Position = projViewMatrix * wv;
}