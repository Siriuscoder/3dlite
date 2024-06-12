#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D AlbedoMask;
uniform float SpecularAmbientFactor;

vec3 sampleSpecular(vec2 iuv);
vec3 sampleNormal(vec2 iuv, mat3 tbn);

vec4 getAlbedo(vec2 uv)
{
    float alphaMask = texture(AlbedoMask, uv).a;
    vec3 albedo = texture(Albedo, uv).rgb;
    return vec4(albedo, alphaMask);
}

vec3 getEmission(vec2 uv)
{
    return vec3(0.0);
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return sampleNormal(uv, tbn);
}

vec3 getSpecular(vec2 uv)
{
    return sampleSpecular(uv);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return SpecularAmbientFactor;
}