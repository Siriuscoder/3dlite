#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 modelMatrix;

out vec2 iuv_g;
flat out int drawID_g;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID_g = gl_DrawIDARB;
    // vertex coordinate in world space 
    gl_Position = invInfo.model * vertex;
#else
    gl_Position = modelMatrix * vertex;
#endif

    iuv_g = uv;
}