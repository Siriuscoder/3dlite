#include "samples:shaders/sources/common/utils_inc.glsl"

#if defined(MRT_WITH_EMISSION_SOLID)

uniform vec4 Emission;
uniform float EmissionStrength;

#else

uniform sampler2D Albedo;
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

#endif

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
#if defined(MRT_WITH_EMISSION_SOLID)

    vec3 albedo = Emission.rgb;
    vec3 emission = Emission.rgb * EmissionStrength;
    vec3 normal = itbn[2];
    vec3 specular = vec3(1.0, 1.0, 0.0);

#else

    // sampling albedo 
    vec3 albedo = texture(Albedo, iuv).rgb;
    // Non emission material 
    vec3 emission = vec3(0.0);
    // Get Fixed normal
    vec3 normal = GetFixedWorldNormal(itbn, iuv);
    // Get Specular params
    vec3 specular = GetSpecular(iuv);

#endif


    fragWCoord = vec4(ivv, 0.10);
    fragWNormal = vec4(normal, emission.r);
    fragAlbedo = vec4(albedo, emission.g);
    fragSpecular = vec4(specular, emission.b);
}