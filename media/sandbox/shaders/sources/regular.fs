#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Roughness;
uniform float Metallic;
uniform float SpecularAmbientFactor;

vec4 getAlbedo(vec2 uv)
{
    return Albedo;
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
    vec3 specular = vec3(1.0, Roughness, Metallic);
    return clamp(specular, 0.0, 1.0);
}

float getSpecularAmbient(vec2 uv)
{
    return 2.2;
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
