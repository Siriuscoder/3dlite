#include "samples:shaders/sources/common/version.def"

uniform samplerCube cube;
uniform vec3 eye;

in vec2 uv;
in vec3 wn;
in vec3 wv;
out vec4 fragcolor;

const vec3 lightDir = vec3(0.0, -1.0, 1.0);

void main()
{
	vec3 reflected = reflect(normalize(eye - wv), wn).xzy;
    vec4 dreflect = texture(cube, reflected);

    float nDotL = clamp(dot(wn, lightDir), 0.32, 1.0);
    vec3 d = dreflect.xyz * nDotL;
    fragcolor = vec4(d, 1.0);
}