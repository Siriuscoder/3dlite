#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 btang;

uniform mat4 modelMatrix;

out vec2 iuv_g;
out vec3 iwn_g;
out vec3 iwt_g;
out vec3 iwb_g;

void main()
{
    // texture coordinate 
    iuv_g = uv;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vertex;
    // calculate tangent, normal, binormal in world space
    mat3 normalMatrix = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    iwt_g = normalize(normalMatrix * tang);
    iwb_g = normalize(normalMatrix * btang);
    iwn_g = normalize(normalMatrix * normal);
}