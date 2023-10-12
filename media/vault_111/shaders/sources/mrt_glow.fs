#ifdef USE_GLOW_SOLID
uniform vec4 Emission;
#else
uniform sampler2D Emission;
#endif

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
#ifdef USE_GLOW_SOLID
    vec4 albedo = Emission;
#else
    // sampling albedo 
    vec4 albedo = texture(Emission, iuv);
#endif

    // get normal from tbn
    vec3 nw = itbn[2];
    // setup specular
    vec4 specular = vec4(Specular, Roughness, 0.0, 0.0);

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(nw, 0.0);
    fragAlbedo = vec4(albedo.rgb, EmissionStrength);
    fragSpecular = specular;
}