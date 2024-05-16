#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
uniform float Metallic;
uniform float Roughness;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);

void main()
{
    // sampling normal and convert to world space
    vec3 nw = itbn[2];
    // sampling specular PBR parameters
    vec3 specular = vec3(1.0, Roughness, Metallic);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, Albedo.rgb, vec3(0.0), specular, 1.0, 1.0);

    fragColor = vec4(total, Albedo.a);
}