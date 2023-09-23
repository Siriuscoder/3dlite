#ifdef USE_GLOW_SOLID
uniform vec4 Emission;
#else
uniform sampler2D Emission;
#endif

uniform float EmissionStrength;
uniform float Roughness;
uniform float Specular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
#ifdef USE_GLOW_SOLID
    vec4 albedo = Emission
#else
    // sampling albedo 
    vec4 albedo = texture(Emission, iuv);
#endif

    // get normal from tbn
    vec3 nw = itbn[2];
    // setup specular
    vec4 specular = vec4(Specular, Roughness, 0.0, 0.0);

    gl_FragData[0] = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    gl_FragData[1] = vec4(nw, 0.0);
    gl_FragData[2] = vec4(albedo.rgb, EmissionStrength);
    gl_FragData[3] = specular;
}