#include "samples:shaders/sources/common/version.def"

uniform samplerCube skybox;
uniform float Strength;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(skybox, iuv).rgb * Strength, 1.0);
}