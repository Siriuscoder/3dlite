#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Alpha;

vec4 getAlbedo(vec2 uv)
{
    return vec4(Emission.rgb, Alpha);
}

vec3 getEmission(vec2 uv)
{
    return Emission.rgb * EmissionStrength;
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return itbn[2];
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
