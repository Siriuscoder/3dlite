#ifdef BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
uniform mat4 modelMatrix;
#endif

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

out vec2 iuv_g;
flat out int drawID_g;

void main()
{
#ifdef BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID_g = gl_DrawIDARB;
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
#else
    gl_Position = modelMatrix * vertex;
#endif

    iuv_g = uv;
}