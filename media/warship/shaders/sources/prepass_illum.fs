#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuse;
uniform sampler2D glow;
uniform sampler2D normals;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

bool vec3zero(vec3 vec)
{
    float prec = 0.000001;
    return ((1.0-step(prec, vec.x)) * (1.0-step(prec, vec.y)) * (1.0-step(prec, vec.z))) == 1.0;
}

void main()
{
    fragAlbedo = vec4(texture(diffuse, iuv).rgb, 0.0);
    vec3 fragGlow = texture(glow, iuv).xyz;
    if (!vec3zero(fragGlow))
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