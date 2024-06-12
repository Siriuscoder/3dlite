#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;

vec4 getAlbedo(vec2 uv)
{
    vec4 a = texture(Albedo, uv);
    return vec4(a.rgb, a.a * 0.68);
}

vec3 getEmission(vec2 uv)
{
    return vec3(0.0);
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return tbn[2];
}

vec3 getSpecular(vec2 uv)
{
    return vec3(1.0, 0.77, 0.0);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return 0.05;
}
