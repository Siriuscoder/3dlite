#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2D fragMap;
uniform sampler2D normalMap;
uniform vec3 eye;

uniform LightSource light;

in vec2 iuv;
out vec4 fragColor;

const float wrapAroundFactor = 0.1;
const float specPower = 40.0;

vec3 phong_blinn_single(vec3 lightDir, vec3 eyeDir, vec3 normal, in LightSource source,
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec);

void main()
{
    if (!hasFlag(light.flags, LITE3D_LIGHT_ENABLED))
        discard;
    /* sampling normal and specular factor (w)*/
    vec4 normal = texture(normalMap, iuv);
    if (isZero(normal.xyz))
        discard;

    /* fragment coordinate */
    vec3 frag = texture(fragMap, iuv).xyz;
    vec3 lightDir = light.position.xyz - frag;
    /* check light distance */
    if (length(lightDir) > light.influenceDistance)
        discard;

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - frag);

    vec3 linearSpec;
    vec3 linear = phong_blinn_single(lightDir, eyeDir, normal.xyz, light,
        normal.w, wrapAroundFactor, specPower, linearSpec);

    /* result color in HDR */
    fragColor = vec4(linear, 1.0);
}