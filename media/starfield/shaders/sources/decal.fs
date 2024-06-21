#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Alpha;
uniform sampler2D Rough;
uniform sampler2D Metal;
uniform sampler2D Normal;

vec4 getAlbedo(vec2 uv)
{
    vec3 albedo = texture(Albedo, uv).rgb;
    float alpha = texture(Alpha, uv).r;
    return vec4(albedo, alpha);
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
    // Refix Z (may be missing)
    nt.z = sqrt(1.0 - dot(nt.xy, nt.xy));
    // Reverse Y
    nt.y *= -1.0;
    // trasform normal to world space using common TBN
    return normalize(tbn * nt);
}

vec3 getSpecular(vec2 uv)
{
    float rough = texture(Rough, uv).r;
    float metal = texture(Metal, uv).r;
    vec3 specular = vec3(1.0, rough, metal);
    return clamp(specular, 0.0, 1.0);
}

float getSpecularAmbient(vec2 uv)
{
    return 0.15;
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
