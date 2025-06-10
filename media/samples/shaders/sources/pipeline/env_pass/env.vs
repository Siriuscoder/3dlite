#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
layout(location = 5) in ivec4 boneIndexes;
layout(location = 6) in vec4 boneWeights;
#endif

uniform mat4 modelMatrix;

out vec2 iuv_g;
out vec3 iwn_g;

void main()
{
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    ChunkInvocationInfo invInfo = getInvocationInfo();

#ifdef LITE3D_VERTEX_SKELETON_DEFORM
    skeletonDeform(invInfo, boneIndexes, boneWeights);
#endif

    mat3 normalMatrix = mat3(invInfo.normalMatrix[0].xyz, invInfo.normalMatrix[1].xyz, invInfo.normalMatrix[2].xyz);
    // vertex coordinate in world space 
    gl_Position = invInfo.modelMatrix * vertex;
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