#include "samples:shaders/sources/common/version.def"

uniform sampler2D fragMap;
uniform sampler2D normalMap;
uniform vec3 eye;

struct lightSource 
{
    int enabled;
    int type;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    vec3 spotfactor;
    vec4 attenuation;
};

uniform lightSource light;

in vec2 iuv;
out vec4 fragColor;

const float wrapAroundFactor = 0.1;
const float specPower = 40.0;

vec3 phong_blinn_single(int type, vec3 lightDir, vec3 eyeDir, vec3 diffuse, 
    vec3 specular, vec3 normal, vec3 spotDirection, vec2 spotFactor, vec3 attenuation, 
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec);

void main()
{
    if (light.enabled == 0)
        discard;
    /* sampling normal and specular factor (w)*/
    vec4 normal = texture2D(normalMap, iuv);
    if (normal.x == 0 && normal.y == 0 && normal.z == 0)
        discard;

    /* fragment coordinate */
    vec3 frag = texture2D(fragMap, iuv).xyz;
    vec3 lightDir = light.position - frag;
    /* check light distance */
    if (length(lightDir) > light.attenuation.w)
        discard;

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - frag);

    vec3 linearSpec;
    vec3 linear = phong_blinn_single(light.type, lightDir, eyeDir, light.diffuse, light.specular, normal.xyz, 
        light.direction, light.spotfactor.xy, light.attenuation.xyz, normal.w, wrapAroundFactor, specPower, linearSpec);

    /* result color in HDR */
    fragColor = vec4(linear, 1.0);
}