#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D AlbedoMask;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular);
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

void main()
{
    /* sampling albedo mask alpha */
    float mask = texture(AlbedoMask, iuv).a;
    /* sampling albedo */
    vec3 albedo = texture(Albedo, iuv).rgb;
    // sampling normal and convert to world space
    vec3 nw = GetFixedWorldNormal(itbn, iuv);
    // sampling specular PBR parameters
    vec3 specular = GetSpecular(iuv);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, albedo, vec3(0.0), specular);

    fragColor = vec4(total, mask);
}
