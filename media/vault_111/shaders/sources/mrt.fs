#include "samples:shaders/sources/common/utils_inc.glsl"

#if defined(MRT_WITH_EMISSION)

uniform sampler2D Albedo;
uniform sampler2D Emission;
uniform float EmissionStrength;

#elif defined(MRT_WITH_EMISSION_SOLID)

uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Roughness;
uniform float Specular;

#else

uniform sampler2D Albedo;

#endif

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

vec3 CheckAlbedo(vec4 albedo)
{
    // check albedo alpha and discard full transparent fragments
    if (fiszero(albedo.a))
        discard;
    
    return albedo.rgb;
}

void main()
{
#if defined(MRT_WITH_EMISSION)
    // sampling albedo 
    vec3 albedo = CheckAlbedo(texture(Albedo, iuv));
    // sampling emission 
    vec3 emission = texture(Emission, iuv).rgb * EmissionStrength;
    // Get Fixed normal
    vec3 normal = GetFixedWorldNormal(itbn, iuv);
    // Get Specular params
    vec3 specular = GetSpecular(iuv);

#elif defined(MRT_WITH_EMISSION_SOLID)

    vec3 albedo = Emission.rgb;
    vec3 emission = Emission.rgb * EmissionStrength;
    vec3 normal = itbn[2];
    vec3 specular = vec3(Specular, Roughness, 0.0);

#else

    // sampling albedo 
    vec3 albedo = CheckAlbedo(texture(Albedo, iuv));
    // Non emission material 
    vec3 emission = vec3(0.0);
    // Get Fixed normal
    vec3 normal = GetFixedWorldNormal(itbn, iuv);
    // Get Specular params
    vec3 specular = GetSpecular(iuv);

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

#ifdef EMISSION_GREEN
    emission *= EMISSION_GREEN;
#endif

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(normal, emission.r);
    fragAlbedo = vec4(albedo, emission.g);
    fragSpecular = vec4(specular, emission.b);
}