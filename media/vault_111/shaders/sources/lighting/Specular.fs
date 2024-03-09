#include "samples:shaders/sources/common/version.def"

uniform sampler2D Specular;

vec3 GetSpecular(vec2 iuv)
{
    vec3 specular = texture(Specular, iuv).rgb;
    specular = vec3(specular.g, 1.0 - specular.g, specular.r * specular.g);
    return specular;
}