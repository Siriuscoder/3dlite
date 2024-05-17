#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
    vec4 albedo = texture(Albedo, iuv);
    // sampling normal and convert to world space
    vec3 nw = itbn[2];
    // sampling specular PBR parameters
    vec3 specular = vec3(1.0, 0.77, 0.0);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, albedo.rgb, vec3(0.0), specular, 1.0, 0.0);

    fragColor = vec4(total, albedo.a * 0.5);
}