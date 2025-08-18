#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
layout(location = 5) in ivec4 boneIndexes;
layout(location = 6) in vec4 boneWeights;
#endif

uniform mat4 projViewMatrix;
uniform mat4 modelMatrix;

out vec2 iuv;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
    skeletonDeform(invInfo, boneIndexes, boneWeights);
#endif

    vec4 wv = invInfo.modelMatrix * vertex;
#else
    vec4 wv = modelMatrix * vertex;
#endif

    iuv = uv;
    gl_Position = projViewMatrix * wv;
}
