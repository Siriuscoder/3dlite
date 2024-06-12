#include "samples:shaders/sources/common/utils_inc.glsl"

#if defined(MRT_WITH_EMISSION)

uniform sampler2D Albedo;
uniform sampler2D Emission;
uniform float EmissionStrength;

#elif defined(MRT_WITH_EMISSION_SOLID)

uniform vec4 Emission;
uniform float EmissionStrength;

#else

uniform sampler2D Albedo;
uniform float SpecularAmbientFactor;

#endif

vec3 sampleSpecular(vec2 iuv);
vec3 sampleNormal(vec2 iuv, mat3 tbn);

vec3 CheckAlbedo(vec4 albedo)
{
    // check albedo alpha and discard full transparent fragments
    if (fiszero(albedo.a))
        discard;
    
    return albedo.rgb;
}

//////////////////////// ALBEDO ///////////////////////////
vec4 getAlbedo(vec2 uv)
{
#if defined(MRT_WITH_EMISSION_SOLID)
    vec3 albedo = Emission.rgb;
#else
    vec3 albedo = CheckAlbedo(texture(Albedo, iuv));
#endif

// Additional modifiers
#ifdef PALETE_YELLOW
    albedo *= PALETE_YELLOW;
#endif

#ifdef PALETE_RED
    albedo *= PALETE_RED;
#endif

#ifdef PALETE_BLUE
    albedo *= PALETE_BLUE;
#endif

    return vec4(albedo, 1.0);
}
///////////////////////// EMISSION ////////////////////////
vec3 getEmission(vec2 uv)
{
#if defined(MRT_WITH_EMISSION)
    vec3 emission = texture(Emission, iuv).rgb * EmissionStrength;
#elif defined(MRT_WITH_EMISSION_SOLID)
    vec3 emission = Emission.rgb * EmissionStrength;
#else
    vec3 emission = vec3(0.0);
#endif

#ifdef EMISSION_GREEN
    emission *= EMISSION_GREEN;
#endif

    return emission;
}
///////////////////////// SPECULAR AMBIENT ////////////////
float getSpecularAmbient(vec2 uv)
{
#if defined(MRT_WITH_EMISSION)
    return 0.1;
#elif defined(MRT_WITH_EMISSION_SOLID)
    return 0.1;
#else
    return SpecularAmbientFactor;
#endif
}
///////////////////////// NORMAL //////////////////////////
vec3 getNormal(vec2 uv, mat3 tbn)
{
#if defined(MRT_WITH_EMISSION_SOLID)
    return itbn[2];
#else
    return sampleNormal(uv, tbn);
#endif
}
///////////////////////// SPECULAR ////////////////////////
vec3 getSpecular(vec2 uv)
{
#if defined(MRT_WITH_EMISSION_SOLID)
    return vec3(0.5, 1.0, 0.0);
#else
    return sampleSpecular(uv);
#endif
}
