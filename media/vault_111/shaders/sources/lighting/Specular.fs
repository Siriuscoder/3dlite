#include "samples:shaders/sources/common/version.def"

uniform sampler2D Specular;

vec3 GetSpecular(vec2 iuv)
{
    vec3 specular = texture(Specular, iuv).rgb;
    specular = vec3(specular.g,
        clamp(1.0 - specular.r, 0.0, 1.0), 
        clamp(1.0 - specular.b, 0.0, 1.0));

    return specular;
}