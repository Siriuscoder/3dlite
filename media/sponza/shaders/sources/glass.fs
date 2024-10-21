uniform vec4 Albedo;
uniform float Metallic;
uniform float Roughness;

vec4 getAlbedo(vec2 uv)
{
    return Albedo;
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
    return vec3(1.0, Roughness, Metallic);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return 2.5;
}
