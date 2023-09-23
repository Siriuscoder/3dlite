#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Emission;
uniform float Roughness;
uniform float Specular;
uniform float EmissionStrength;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    vec4 mask = texture(Albedo, iuv);
    vec4 albedo = texture(Emission, iuv);
    gl_FragColor = vec4(albedo.rgb, mask.w);
}