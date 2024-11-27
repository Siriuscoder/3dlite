#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
uniform mat4 modelMatrix;
#endif

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec2 iuv_g;
out vec3 iwn_g;
flat out int drawID_g;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID_g = gl_DrawIDARB;
    mat3 normalMatrix = mat3(invInfo.normal[0].xyz, invInfo.normal[1].xyz, invInfo.normal[2].xyz);
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
#else 
    mat3 normalMatrix = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vertex;
#endif

    // texture coordinate 
    iuv_g = uv;
    // calculate normal in world space
    iwn_g = normalize(normalMatrix * normal);
}