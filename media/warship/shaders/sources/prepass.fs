#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuse;
uniform sampler2D normals;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    // sampling normal from normal map
    vec4 nval = texture(normals, iuv);
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2*nval.rgb-1));
    // sampling diffuse color 
    vec4 fragDiffuse = texture(diffuse, iuv);

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(nw, nval.a);
    fragAlbedo = vec4(fragDiffuse.rgb, 0.0);
}