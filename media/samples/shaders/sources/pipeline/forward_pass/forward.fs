#include "samples:shaders/sources/common/utils_inc.glsl"

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

// You must implement this methods in you shader to provide data for forward pass
vec4 getAlbedo(vec2 uv);
vec3 getEmission(vec2 uv);
vec3 getNormal(vec2 uv, mat3 tbn);
vec3 getSpecular(vec2 uv);
float getAmbientOcclusion(vec2 uv);
float getSpecularAmbient(vec2 uv);

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
#if defined(TBN_FROM_NORMAL)
    mat3 tbn = TBN(iwn);
#elif defined(TBN_FROM_NORMAL_TANGENT)
    mat3 tbn = TBN(iwn, iwt);
#else
    mat3 tbn = TBN(iwn, iwt, iwb);
#endif

    // Get albedo 
    vec4 albedo = getAlbedo(iuv);
    // Get emission  
    vec3 emission = getEmission(iuv);
    // Get world-space normal
    vec3 normal = getNormal(iuv, tbn);
    // Get Specular params
    vec3 specular = getSpecular(iuv);
    // Compute total illumination 
    vec3 total = ComputeIllumination(iwv, normal, albedo.rgb, emission, specular, 
        getAmbientOcclusion(iuv), getSpecularAmbient(iuv));
    // Final
    fragColor = vec4(total, albedo.a);
}
