#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor);
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

void main()
{
    /* sampling albedo with alpha */
    vec4 albedo = texture(Albedo, iuv);
    // sampling normal and convert to world space
    vec3 nw = GetFixedWorldNormal(itbn, iuv);
    // sampling specular PBR parameters
    vec3 specular = GetSpecular(iuv);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, albedo.rgb, vec3(0.0), specular, 1.0);

    fragColor = vec4(total, albedo.a);
}