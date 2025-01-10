#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 projViewMatrix;
uniform mat4 modelMatrix;

out vec2 iuv;
flat out int drawID;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID = gl_DrawIDARB;
    vec4 wv = invInfo.modelMatrix * vertex;
#else
    vec4 wv = modelMatrix * vertex;
#endif

    iuv = uv;
    gl_Position = projViewMatrix * wv;
}
