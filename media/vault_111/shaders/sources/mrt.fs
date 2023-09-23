#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Normal;
uniform sampler2D Specular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    // sampling normal from normal map
    vec4 n = texture(Normal, iuv);
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize((2.0 * n.rgb) - 1.0));
    // sampling albedo 
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular
    vec4 specular = texture(Specular, iuv);
    // Transform Specular, Roughness, Metalic 
    specular = vec4(specular.x, 1.0 - specular.y, 1.0 - specular.z, 0.0);

    gl_FragData[0] = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    gl_FragData[1] = vec4(nw, 0.0);
    gl_FragData[2] = vec4(albedo.rgb, 1.0);
    gl_FragData[3] = specular;
}