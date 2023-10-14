#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Specular;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 specular, float emissionStrength);
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);

void main()
{
    /* sampling albedo with alpha */
    vec4 albedo = texture(Albedo, iuv);
    // sampling normal and convert to world space
    vec3 nw = GetFixedWorldNormal(itbn, iuv);
    // sampling specular PBR parameters
    vec3 specular = texture(Specular, iuv).xyz;
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, albedo.rgb, specular, 0.0);

    fragColor = vec4(total, albedo.a);
}