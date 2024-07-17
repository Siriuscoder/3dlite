#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 btang;
layout(location = 5) in mat4 modelMatrix;

uniform mat4 projViewMatrix;

out vec2 iuv;
out vec3 iwv;
out vec3 iwn;
out vec3 iwt;
out vec3 iwb;

void main()
{

    vec4 wv = modelMatrix * vertex;
    // vertex coordinate in world space 
    iwv = wv.xyz / wv.w;
    // texture coordinate 
    iuv = uv;
    // calculate tangent, normal, binormal in world space
    mat3 normalMatrix = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    iwt = normalize(normalMatrix * tang);
    iwb = normalize(normalMatrix * btang);
    iwn = normalize(normalMatrix * normal);

    gl_Position = projViewMatrix * wv;
}