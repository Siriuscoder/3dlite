#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float EmissionStrength;

vec4 getAlbedo(vec2 uv)
{
    return vec4(Emission.rgb, 1.0);
}

vec3 getEmission(vec2 uv)
{
    return Emission.rgb * EmissionStrength;
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return tbn[2];
}

vec3 getSpecular(vec2 uv)
{
    return vec3(1.0, 1.0, 0.0);
}

float getSpecularAmbient(vec2 uv)
{
    return 1.3;
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
