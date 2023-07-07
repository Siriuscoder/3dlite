#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuseSampler;

in vec2 uv;
out vec4 fragcolor;

void main()
{
    fragcolor = texture2D(diffuseSampler, uv.st);
}