#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
uniform sampler2D AlbedoMask;
uniform float Specular;
uniform float Roughness;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 specular, float emissionStrength);

void main()
{
    /* sampling albedo with alpha */
    float mask = texture(AlbedoMask, iuv).a;
    // get normal from TBN
    vec3 nw = itbn[2];
    // specular PBR parameters 
    vec3 specular = vec3(Specular, Roughness, 0.0);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, Albedo.rgb, specular, 0.0);

    fragColor = vec4(total, mask);
}
