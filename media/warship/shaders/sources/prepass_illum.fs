#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuse;
uniform sampler2D glow;
uniform sampler2D normals;

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
    vec4 fragDiffuse = vec4(texture(diffuse, iuv).rgb, 0.0);
    vec3 fragGlow = texture(glow, iuv).xyz;
    if (!vec3zero(fragGlow))
    {
        fragDiffuse = vec4(fragGlow, 1.0);
    }


    // sampling normal from normal map
    vec4 nval = texture(normals, iuv);
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2*nval.rgb-1));

    gl_FragData[0] = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    gl_FragData[1] = vec4(nw, nval.a);
    gl_FragData[2] = fragDiffuse;
}