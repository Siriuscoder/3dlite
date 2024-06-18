#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

vec4 getAlbedo(vec2 uv)
{
    return texture(Albedo, uv);
}
