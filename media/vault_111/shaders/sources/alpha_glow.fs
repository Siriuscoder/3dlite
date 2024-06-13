#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Emission;
uniform float EmissionStrength;

vec4 getAlbedo(vec2 uv)
{
    return texture(Albedo, uv);
}

vec3 getEmission(vec2 uv)
{
    return texture(Emission, uv).rgb * EmissionStrength;
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return tbn[2];
}

vec3 getSpecular(vec2 uv)
{
    return vec3(0.5, 1.0, 0.0);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return 0.1;
}