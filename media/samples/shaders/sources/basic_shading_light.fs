#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

in vec2 uv;
in vec3 wn;
out vec4 fragcolor;

void main()
{
    vec3 lightDir = vec3(0.0, 1.0, 1.0);
    float nDotL = clamp(dot(wn, lightDir), 0.2, 1.0);
    vec4 d = texture(Albedo, uv.st);
    fragcolor = vec4(d.xyz * nDotL, d.w);
}