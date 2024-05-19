#include "samples:shaders/sources/common/version.def"

uniform samplerCube skybox;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(skybox, iuv).rgb * 25.0, 1.0);
}