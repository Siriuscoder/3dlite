#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

in vec2 uv;
out vec4 fragcolor;

void main()
{
    fragcolor = texture(Albedo, uv.st);
}