#include "samples:shaders/sources/common/utils_inc.glsl"

layout(location = 0) out vec4 channel01;
layout(location = 1) out vec4 channel02;
layout(location = 2) out vec4 channel03;
layout(location = 3) out vec4 channel04;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

// You must implement this methods in you shader to provide data for defered pass
vec4 getAlbedo(vec2 uv);
vec3 getEmission(vec2 uv);
vec3 getNormal(vec2 uv, mat3 tbn);
vec3 getSpecular(vec2 uv);
float getSpecularAmbient(vec2 uv);

void main()
{
    // Get world-space normal
#if defined(NORMAL_MAPPING_OFF)
    vec3 normal = normalize(iwn);
#elif defined(NORMAL_MAPPING_TANGENT)
    vec3 normal = getNormal(iuv, TBN(iwn, iwt));
#else
    vec3 normal = getNormal(iuv, TBN(iwn, iwt, iwb));
#endif

    // Get albedo 
    vec3 albedo = getAlbedo(iuv).rgb;
    // Get emission  
    vec3 emission = getEmission(iuv);
    // Get Specular params
    vec3 specular = getSpecular(iuv);

    channel01 = vec4(iwv, getSpecularAmbient(iuv));
    channel02 = vec4(normal, emission.r);
    channel03 = vec4(albedo, emission.g);
    channel04 = vec4(specular, emission.b);
}