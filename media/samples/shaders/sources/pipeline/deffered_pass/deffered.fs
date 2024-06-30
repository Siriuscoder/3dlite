#include "samples:shaders/sources/common/version.def"

layout(location = 0) out vec4 channel01;
layout(location = 1) out vec4 channel02;
layout(location = 2) out vec4 channel03;
layout(location = 3) out vec4 channel04;

in vec2 iuv;    // UVs
in vec4 ivv;    // world-space position
in mat3 itbn;   // The matrix that transforms vector from tangent space to world space 

// You must implement this methods in you shader to provide data for defered pass
vec4 getAlbedo(vec2 uv);
vec3 getEmission(vec2 uv);
vec3 getNormal(vec2 uv, mat3 tbn);
vec3 getSpecular(vec2 uv);
float getSpecularAmbient(vec2 uv);

void main()
{
    vec3 P = ivv.xyz / ivv.w;
    // Get albedo 
    vec3 albedo = getAlbedo(iuv).rgb;
    // Get emission  
    vec3 emission = getEmission(iuv);
    // Get world-space normal
    vec3 normal = getNormal(iuv, itbn);
    // Get Specular params
    vec3 specular = getSpecular(iuv);

    channel01 = vec4(P, getSpecularAmbient(iuv));
    channel02 = vec4(normal, emission.r);
    channel03 = vec4(albedo, emission.g);
    channel04 = vec4(specular, emission.b);
}