#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float Specular;
uniform float Roughness;
uniform float EmissionStrength;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular);

void main()
{
    float alpha = smoothstep(mix(0.01, 0.5, iuv.x), 0.8, iuv.x) / 10.0;
    // get normal from TBN
    vec3 nw = itbn[2];
    // specular PBR parameters 
    vec3 specular = vec3(Specular, Roughness, 0.0);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, Emission.rgb, Emission.rgb * EmissionStrength, specular);

    fragColor = vec4(total, alpha);
}
