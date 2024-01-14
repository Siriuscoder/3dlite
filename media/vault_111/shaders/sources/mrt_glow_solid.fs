#include "samples:shaders/sources/common/version.def"

uniform vec4 Emission;
uniform float EmissionStrength;
uniform float Roughness;
uniform float Specular;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    vec3 albedo = Emission.rgb;
    vec3 emission = Emission.rgb * EmissionStrength;

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(itbn[2], emission.r);
    fragAlbedo = vec4(albedo, emission.g);
    fragSpecular = vec4(Specular, Roughness, 0.0, emission.b);
}