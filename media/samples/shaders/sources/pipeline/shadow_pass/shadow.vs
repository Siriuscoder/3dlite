#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
layout(location = 5) in ivec4 boneIndexes;
layout(location = 6) in vec4 boneWeights;
#endif

uniform mat4 modelMatrix;

out vec2 iuv_g;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
    skeletonDeform(invInfo, boneIndexes, boneWeights);
#endif

    // vertex coordinate in world space 
    gl_Position = invInfo.modelMatrix * vertex;
#else
    gl_Position = modelMatrix * vertex;
#endif

    iuv_g = uv;
}