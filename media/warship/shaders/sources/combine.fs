#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/lighting/lighting_inc.glsl"

uniform sampler2D fragMap;
uniform sampler2D normalMap;
uniform sampler2D diffuseMap;
uniform vec3 eye;

out vec4 fragColor;
in vec2 iuv;

const vec3 ambient = vec3(0.02, 0.02, 0.02);
const float wrapAroundFactor = 0.1;
const float specPower = 40.0;
const float density = 0.0005;
const float LOG2 = 1.442695;
const vec3 fogColor = vec3(0.5, 0.5, 0.2);

void main()
{
    /* fragment coordinate */
    vec4 fragXYZW = texture(fragMap, iuv);
    vec4 fragTexture = texture(diffuseMap, iuv);
    /* check fragment not shaded or self-illum material */
    if (fragXYZW.w == 1.0 || fragTexture.w == 1.0)
    {
        fragColor = vec4(fragTexture.xyz, 1.0);
        return;
    }
    /* sampling normal and specular factor (w)*/
    vec4 fragNormalAndSpecular = texture(normalMap, iuv);

    vec3 linearSpec = vec3(0.0);
    vec3 linear = calc_lighting(fragXYZW.xyz, fragNormalAndSpecular.xyz,
        eye, fragNormalAndSpecular.w * 2.0, wrapAroundFactor, specPower, linearSpec);

    /* calculate fog factor */
    float fogFactor = clamp(exp2(-density * density * fragXYZW.w * fragXYZW.w * LOG2), 0.0, 1.0);
    vec3 rcolor = mix(fogColor, (ambient + linear) * fragTexture.rgb, fogFactor);
    /* result color in LDR */
    fragColor = vec4(rcolor, 1.0);
}