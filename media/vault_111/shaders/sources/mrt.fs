#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Specular;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);

void main()
{
    // sampling albedo 
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular
    vec4 specular = texture(Specular, iuv);
    // Transform Specular, Roughness, Metalic 
    specular = vec4(specular.g,
        clamp(1.0 - specular.r, 0.0, 1.0), 
        clamp(1.0 - specular.b, 0.0, 1.0), 
        0.0);

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(GetFixedWorldNormal(itbn, iuv), 0.0);
    fragAlbedo = vec4(albedo.rgb, 0.0);
    fragSpecular = specular;
}