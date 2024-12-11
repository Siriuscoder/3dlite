#include "samples:shaders/sources/common/common_inc.glsl"

layout(std430) buffer readonly lightSources 
{
    LightSource lights[];
};

layout(std430) buffer readonly lightIndexes 
{
    ivec4 lightsIndexes[];
};

vec3 phong_blinn_single(vec3 lightDir, vec3 eyeDir, vec3 normal, in LightSource source,
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec);

vec3 calc_lighting(vec3 fragPos, 
    vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 linear = vec3(0.0);
    linearSpec = vec3(0.0);
    vec3 lightDir = vec3(0.0);

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - fragPos);

    int count = lightsIndexes[0].x;
    for(int i = 1; i <= count; i++)
    {
        int index = lightsIndexes[i/4][int(mod(i, 4))];
        LightSource light = lights[index];

        if (!hasFlag(light.flags, LITE3D_LIGHT_ENABLED))
            continue;

        /* Read Position and check distance for spot and point light only */
        if (hasFlag(light.flags, LITE3D_LIGHT_POINT) || hasFlag(light.flags, LITE3D_LIGHT_SPOT))
        {
            /* calculate direction from fragment to light */
            lightDir = light.position.xyz - fragPos;
            /* distance to light */
            float dist = length(lightDir);
            /* check light distance */
            if (dist > light.influenceDistance)
                continue;
        }

        vec3 curSpec = vec3(0.0);
        linear += phong_blinn_single(lightDir, eyeDir, fragNormal, light, 
            specularFactor, wrapAroundFactor, specPower, curSpec);
        linearSpec += curSpec;
    }

    return linear;
}
