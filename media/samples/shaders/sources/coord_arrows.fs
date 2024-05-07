#include "samples:shaders/sources/common/version.def"

in vec3 icolor;
out vec4 fragcolor;

void main()
{
    fragcolor = vec4(icolor, 1.0);
}