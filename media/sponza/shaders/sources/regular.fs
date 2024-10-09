#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform sampler2D Normal;

vec4 getAlbedo(vec2 uv)
{
    return vec4(texture(Albedo, uv).rgb, 1.0);
}

vec3 getEmission(vec2 uv)
{
    return vec3(0.0);
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    // sampling normal from normal map with swap XY
    vec3 nt = texture(Normal, uv).rgb;
    // put normal in [-1,1] range in tangent space
    nt = 2.0 * clamp(nt, 0.0, 1.0) - 1.0;
    // Reverse Y
    nt.y *= -1.0;
    // trasform normal to world space using common TBN
    return normalize(tbn * normalize(nt));
}

vec3 getSpecular(vec2 uv)
{
    vec3 specular = vec3(1.0, texture(Specular, uv).gb);
    return clamp(specular, 0.0, 1.0);
}

float getSpecularAmbient(vec2 uv)
{
    return 1.1;
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
