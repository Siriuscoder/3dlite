#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2D diffuse;
uniform sampler2D glow;
uniform sampler2D normals;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    fragAlbedo = vec4(texture(diffuse, iuv).rgb, 0.0);
    vec3 fragGlow = texture(glow, iuv).xyz;
    if (!isZero(fragGlow))
    {
        fragAlbedo = vec4(fragGlow, 1.0);
    }


    // sampling normal from normal map
    vec4 nval = texture(normals, iuv);
    nval.y = 1.0 - nval.y;
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2.0 * nval.rgb - 1.0));

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(nw, nval.a);
}