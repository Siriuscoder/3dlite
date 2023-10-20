uniform sampler2D Albedo;

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
    vec3 albedo = texture(Albedo, iuv).rgb;

#ifdef PALETE_YELLOW
    albedo *= PALETE_YELLOW;
#endif

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(GetFixedWorldNormal(itbn, iuv), 0.0);
    fragAlbedo = vec4(albedo, 0.0);
    fragSpecular = vec4(GetSpecular(iuv), 0.0);
}