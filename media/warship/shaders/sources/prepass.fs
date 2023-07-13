#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuse;
uniform sampler2D normals;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    // sampling normal from normal map
    vec4 nval = texture2D(normals, iuv);
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2*nval.rgb-1));
    // sampling diffuse color 
    vec4 fragDiffuse = texture2D(diffuse, iuv);

    gl_FragData[0] = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    gl_FragData[1] = vec4(nw, nval.a);
    gl_FragData[2] = vec4(fragDiffuse.rgb, 0.0);
}