#include "samples:shaders/sources/common/version.def"

uniform vec4 diffuse;
out vec4 fragcolor;

void main()
{
    fragcolor = diffuse;
}