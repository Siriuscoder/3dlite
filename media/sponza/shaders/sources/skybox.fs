#include "samples:shaders/sources/common/version.def"

uniform samplerCube skybox;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    fragColor = texture(skybox, iuv);
}