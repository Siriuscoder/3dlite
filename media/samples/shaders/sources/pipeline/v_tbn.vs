#include "samples:shaders/sources/common/common_inc.glsl"

#ifndef LITE3D_BINDLESS_TEXTURE_PIPELINE
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
#endif

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
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID = gl_DrawIDARB + gl_InstanceID;
    vec4 wv = invInfo.modelMatrix * vertex;
    mat3 normalMatrix = mat3(invInfo.normalMatrix[0].xyz, invInfo.normalMatrix[1].xyz, invInfo.normalMatrix[2].xyz);
#else
    vec4 wv = modelMatrix * vertex;
#endif

    // vertex coordinate in world space 
    iwv = wv.xyz / wv.w;
    // texture coordinate 
    iuv = uv;
    // calculate tangent, normal, binormal in world space
    iwt = normalize(normalMatrix * tang);
    iwb = normalize(normalMatrix * btang);
    iwn = normalize(normalMatrix * normal);

    gl_Position = projViewMatrix * wv;
}