#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Roughness;
uniform float Specular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    gl_FragColor = vec4(Emission.rgb, 1.0);
}