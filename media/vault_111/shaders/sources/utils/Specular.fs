#include "samples:shaders/sources/common/version.def"

uniform sampler2D Specular;
uniform float SpecularFactor;
uniform float MetallicFactor;

vec3 sampleSpecular(vec2 uv)
{
    vec3 specular = texture(Specular, uv).rgb;
    specular = vec3(specular.r * SpecularFactor, clamp(1.0 - specular.g, 0.05, 1.0), specular.r * MetallicFactor);
    return clamp(specular, 0.0, 1.0);
}