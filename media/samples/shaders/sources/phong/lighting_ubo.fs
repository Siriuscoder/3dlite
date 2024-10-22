#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/phong/lighting_inc.glsl"

// declaration for UBO
layout(std140) uniform lightSources
{
    vec4 lights[LITE3D_MAX_LIGHT_COUNT];
};

layout(std140) uniform lightIndexes
{
    ivec4 indexes[LITE3D_MAX_LIGHT_COUNT];
};

vec3 calc_lighting(vec3 fragPos, vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 linear = vec3(0.0);
    linearSpec = vec3(0.0);
    vec3 lightDir = vec3(0.0);

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - fragPos);

    int count = indexes[0].x;
    for(int i = 0; i < count; i++)
    {
        int index = indexes[(i+1)/4][int(mod(i+1, 4))] * 5;

        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        if (isZero(lights[index+0].y))
            continue;

        /* Read Position and check distance for spot and point light only */
        if (isNear(lights[index+0].x, LITE3D_LIGHT_POINT) || isNear(lights[index+0].x, LITE3D_LIGHT_SPOT))
        {
            /* block2.x - position.x */
            /* block2.y - position.y */
            /* block2.z - position.z */
            /* block2.w - size */
    
            /* light position */
            vec3 lposition = lights[index+2].xyz;
            /* calculate direction from fragment to light */
            lightDir = lposition - fragPos;
            /* distance to light */
            float dist = length(lightDir);
            /* check light distance */
            if (dist > lights[index+0].z)
                continue;
        }

        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        vec3 ldiffuse = lights[index+1].xyz;

        /* block3.x - direction.x */
        /* block3.y - direction.y */
        /* block3.z - direction.z */
        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = lights[index+3].xyz;
        
        /* block4.z - angle inner cone */
        /* block4.w - angle outer cone */
        vec2 lspotFactor = lights[index+4].zw;

        /* attenuation factor */
        /* block3.w - attenuation constant */
        /* block4.x - attenuation linear */
        /* block4.y - attenuation quadratic */
        vec3 lattenuation = vec3(lights[index+3].w, lights[index+4].xy);

        vec3 curSpec = vec3(0.0);
        linear += phong_blinn_single(lights[index+0].x, lightDir, eyeDir, ldiffuse, fragNormal, lspotDirection, lspotFactor, 
            lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
        linearSpec += curSpec;
    }

    return linear;
}