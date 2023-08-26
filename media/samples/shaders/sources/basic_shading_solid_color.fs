#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
out vec4 fragcolor;

void main()
{
    fragcolor = Albedo;
}