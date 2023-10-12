#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Roughness;
uniform float Specular;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    fragColor = vec4(Emission.rgb, 1.0);
}