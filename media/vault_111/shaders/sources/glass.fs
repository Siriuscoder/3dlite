uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform float SpecularFactor;
uniform float SpecularAmbientFactor;

vec3 sampleNormal(vec2 iuv, mat3 tbn);

vec4 getAlbedo(vec2 uv)
{
    return texture(Albedo, uv);
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
    vec2 specular = texture(Specular, uv).rg;
    float roughness = 1.0 - specular.g;
    return vec3(specular.r * SpecularFactor, roughness * roughness, 0.0);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return SpecularAmbientFactor;
}

float getIOR(vec2 uv)
{
    return 1.5;
}

float getSheen(vec2 uv)
{
    return 0.0;
}
