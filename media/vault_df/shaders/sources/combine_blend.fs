#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/lighting/lighting_inc.glsl"

uniform sampler2D diffuse;
uniform sampler2D normals;
#ifdef CALC_ILLUM
uniform sampler2D glow;
#endif

uniform vec3 eye;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;
in vec3 wnorm;

out vec4 fragColor;

const float wrapAroundFactor = 0.1;
const vec3 ambient = vec3(0.07, 0.07, 0.07);
#ifdef GLASS
const float specPower = 180.0;
#else
const float specPower = 40.0;
#endif

void main()
{
    vec4 fragTexture = texture2D(diffuse, iuv);
#ifdef CALC_ILLUM
    // sampling glow texture and check colors
    vec3 fragGlow = texture2D(glow, iuv).rgb;
    if (!fiszero(fragGlow))
    {
        fragColor = vec4(fragGlow, fragTexture.a);
        return;
    }
#endif

    /* sampling normal and specular factor (w)*/
    vec4 fragNormalAndSpecular = texture2D(normals, iuv);
#ifdef GLASS
    fragNormalAndSpecular.w = 2.5;
#endif

    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2*fragNormalAndSpecular.xyz-1));
    // fix bad normals
    if (fiszero(fragNormalAndSpecular.xyz))
        nw = wnorm;

    vec3 linearSpec;
    vec3 linear = calc_lighting(ivv, nw,
        eye, fragNormalAndSpecular.w, wrapAroundFactor, specPower, linearSpec);

#ifdef GLASS
    fragTexture.a = mix(fragTexture.a, 1.0, length(linearSpec.rgb));
#endif

    fragColor = vec4((ambient + linear) * fragTexture.rgb, fragTexture.a);
}