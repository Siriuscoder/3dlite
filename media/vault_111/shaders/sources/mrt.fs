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
    // put normal in [-1,1] range in tangent space
    vec3 n = normalize(2.0 * (1.0 - texture(Normal, iuv).rgb) - 1.0);
    // Calculate missing blue channel
	n.b = sqrt(1.0 - dot(n.rg, n.rg));
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * n);
    // sampling albedo 
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular
    vec4 specular = texture(Specular, iuv);
    // Transform Specular, Roughness, Metalic 
    specular = vec4(specular.g,
        clamp(1.0 - specular.r, 0.0, 1.0), 
        clamp(1.0 - specular.b, 0.0, 1.0), 
        0.0);

    gl_FragData[0] = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    gl_FragData[1] = vec4(nw, 0.0);
    gl_FragData[2] = vec4(albedo.rgb, 0.0);
    gl_FragData[3] = specular;
}