#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 btang;
layout(location = 5) in mat4 modelMatrix;

out vec2 iuv_g;
out mat3 itbn_g;

void main()
{
    // texture coordinate 
    iuv_g = uv;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vertex;
    // calculate tangent, normal, binormal in world space
    mat3 normalMatrix = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    itbn_g[0] = normalize(normalMatrix * tang);
    itbn_g[1] = normalize(normalMatrix * btang);
    itbn_g[2] = normalize(normalMatrix * normal);
}