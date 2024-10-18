#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/phong/lighting_inc.glsl"

struct ligthInfo 
{
    vec4 blocks[5];
};

layout(std430) buffer lightSources 
{
    readonly ligthInfo lights[];
};

layout(std430) buffer lightIndexes 
{
    readonly int indexes[];
};

vec3 calc_lighting(vec3 fragPos, 
    vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 linear = vec3(0.0);
    linearSpec = vec3(0.0);
    vec3 lightDir = vec3(0.0);

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - fragPos);

    int count = indexes[0];
    for (int i = 1; i <= count; i++)
    {
        int index = indexes[i];
        ligthInfo light = lights[index];
        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        if (isNear(light.blocks[0].y, 0.0))
            continue;

        /* Read Position and check distance for spot and point light only */
        if (isNear(light.blocks[0].x, LITE3D_LIGHT_POINT) || isNear(light.blocks[0].x, LITE3D_LIGHT_SPOT))
        {
            /* block2.x - position.x */
            /* block2.y - position.y */
            /* block2.z - position.z */
            /* block2.w - size */
    
            /* light position */
            vec3 lposition = light.blocks[2].xyz;
            /* calculate direction from fragment to light */
            lightDir = lposition - fragPos;
            /* distance to light */
            float dist = length(lightDir);
            /* check light distance */
            if (dist > light.blocks[0].z)
                continue;
        }

        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        vec3 ldiffuse = light.blocks[1].xyz;

        /* block3.x - direction.x */
        /* block3.y - direction.y */
        /* block3.z - direction.z */
        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = light.blocks[3].xyz;

        /* block4.z - angle inner cone */
        /* block4.w - angle outer cone */
        vec2 lspotFactor = light.blocks[4].zw;

        /* block3.w - attenuation constant */
        /* block4.x - attenuation linear */
        /* block4.y - attenuation quadratic */
        /* attenuation factor */
        vec3 lattenuation = vec3(light.blocks[3].w, light.blocks[4].xy);

        vec3 curSpec = vec3(0.0);
        linear += phong_blinn_single(light.blocks[0].x, lightDir, eyeDir, ldiffuse, fragNormal, 
            lspotDirection, lspotFactor, lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
        linearSpec += curSpec;
    }

    return linear;
}