#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in vec3 btang;

uniform mat4 screenMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec2 iuv;
out vec3 ivv;
out mat3 itbn;

void main()
{
    // texture coordinate 
    iuv = vec2(uv.x, 1.0 - uv.y);
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vertex;
    ivv = wv.xyz / wv.w;
    // calculate tangent, normal, binormal in world space
    vec3 wTang = normalize(normalMatrix * tang);
    vec3 wBTang = normalize(normalMatrix * btang);
    vec3 wNorm = normalize(normalMatrix * normal);
    // re-orthogonalize T with respect to N
    // wTang = normalize(wTang - dot(wTang, wNorm) * wNorm);
    // vec3 wBTang = cross(wNorm, wTang);
    // TBN matrix to transform normal from tangent space to world space
    itbn = mat3(wTang, wBTang, wNorm);
    
    gl_Position = screenMatrix * vertex;
}