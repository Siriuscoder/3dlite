#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/phong/lighting_inc.glsl"

uniform sampler2D fragMap;
uniform sampler2D normalMap;
uniform vec3 eye;

struct lightSource 
{
    int enabled;
    int type;
    vec3 position;
    vec3 diffuse;
    vec3 direction;
    float influenceDistance;
    float attenuationContant;
    float attenuationLinear;
    float attenuationQuadratic;
    float innercone;
    float outercone;
};

uniform lightSource light;

in vec2 iuv;
out vec4 fragColor;

const float wrapAroundFactor = 0.1;
const float specPower = 40.0;

void main()
{
    if (light.enabled == 0)
        discard;
    /* sampling normal and specular factor (w)*/
    vec4 normal = texture(normalMap, iuv);
    if (isZero(normal.xyz))
        discard;

    /* fragment coordinate */
    vec3 frag = texture(fragMap, iuv).xyz;
    vec3 lightDir = light.position - frag;
    /* check light distance */
    if (length(lightDir) > light.influenceDistance)
        discard;

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - frag);

    vec3 linearSpec;
    vec3 linear = phong_blinn_single(float(light.type), lightDir, eyeDir, light.diffuse, normal.xyz, 
        light.direction, vec2(light.innercone, light.outercone), 
        vec3(light.attenuationContant, light.attenuationLinear, light.attenuationQuadratic), 
        normal.w, wrapAroundFactor, specPower, linearSpec);

    /* result color in HDR */
    fragColor = vec4(linear, 1.0);
}