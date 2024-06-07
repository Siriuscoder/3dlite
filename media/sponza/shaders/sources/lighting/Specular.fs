#include "samples:shaders/sources/common/version.def"

uniform sampler2D Specular;

vec3 GetSpecular(vec2 iuv)
{
    vec3 specular = vec3(1.0, texture(Specular, iuv).gb);
    return clamp(specular, 0.0, 1.0);
}