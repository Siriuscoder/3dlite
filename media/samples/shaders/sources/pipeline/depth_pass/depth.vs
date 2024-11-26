#ifdef BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 projViewMatrix;
uniform mat4 modelMatrix;

out vec2 iuv;
flat out int drawID;

void main()
{
#ifdef BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();
    drawID = gl_DrawIDARB;
    vec4 wv = invInfo.model * vertex;
#else
    vec4 wv = modelMatrix * vertex;
#endif

    iuv = uv;
    gl_Position = projViewMatrix * wv;
}
