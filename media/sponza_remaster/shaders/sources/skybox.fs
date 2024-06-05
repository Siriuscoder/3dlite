#include "samples:shaders/sources/common/version.def"

uniform samplerCube Skybox;
uniform float EmissionStrength;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(skybox, iuv).rgb * EmissionStrength, 1.0);
}