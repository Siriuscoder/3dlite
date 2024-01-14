#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Emission;

uniform float EmissionStrength;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

void main()
{
    // sampling albedo 
    vec4 albedo = texture(Albedo, iuv);
    // sampling emission 
    vec3 emission = texture(Emission, iuv).rgb * EmissionStrength;

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(GetFixedWorldNormal(itbn, iuv), emission.r);
    fragAlbedo = vec4(albedo.rgb, emission.g);
    fragSpecular = vec4(GetSpecular(iuv), emission.b);
}