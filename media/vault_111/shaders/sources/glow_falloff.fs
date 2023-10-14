#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float Specular;
uniform float Roughness;
uniform float EmissionStrength;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 specular, float emissionStrength);
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);

void main()
{
    float alpha = smoothstep(0.2, 1.0, iuv.x) / 2.2;
    // get normal from TBN
    vec3 nw = itbn[2];
    // specular PBR parameters 
    vec3 specular = vec3(Specular, Roughness, 0.0);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, Emission.rgb, specular, EmissionStrength);

    fragColor = vec4(total, alpha);
}
