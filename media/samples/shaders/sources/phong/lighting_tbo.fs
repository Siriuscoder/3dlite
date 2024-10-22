#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/phong/lighting_inc.glsl"

uniform samplerBuffer lightSources;
uniform isamplerBuffer lightIndexes;

vec3 calc_lighting(vec3 fragPos, vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 linear = vec3(0.0);
    linearSpec = vec3(0.0);
    vec3 lightDir = vec3(0.0);
    vec4 blocks[5];

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - fragPos);

    int count = texelFetch(lightIndexes, 0).r;
    for (int i = 0; i < count; i++)
    {
        int texel = texelFetch(lightIndexes, i+1).r * 5;

        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        blocks[0] = texelFetch(lightSources, texel + 0);
        if (isZero(blocks[0].y))
            continue;

        /* Read Position and check distance for spot and point light only */
        if (isNear(blocks[0].x, LITE3D_LIGHT_POINT) || isNear(blocks[0].x, LITE3D_LIGHT_SPOT))
        {
            /* block2.x - position.x */
            /* block2.y - position.y */
            /* block2.z - position.z */
            /* block2.w - size */
            blocks[2] = texelFetch(lightSources, texel + 2);
    
            /* light position */
            vec3 lposition = blocks[2].xyz;
            /* calculate direction from fragment to light */
            lightDir = lposition - fragPos;
            /* distance to light */
            float dist = length(lightDir);
            /* check light distance */
            if (dist > blocks[0].z)
                continue;
        }
        else
        {
            blocks[2] = vec4(0.0);
        }
    
        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        blocks[1] = texelFetch(lightSources, texel + 1);
        /* block3.x - direction.x */
        /* block3.y - direction.y */
        /* block3.z - direction.z */
        /* block3.w - attenuation constant */
        blocks[3] = texelFetch(lightSources, texel + 3);
        /* block4.x - attenuation linear */
        /* block4.y - attenuation quadratic */
        /* block4.z - angle inner cone */
        /* block4.w - angle outer cone */
        blocks[4] = texelFetch(lightSources, texel + 4);

        /* diffuse color */
        vec3 ldiffuse = blocks[1].xyz;

        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = blocks[3].xyz;
        /* x - spot cutoff */
        /* y - spot cutoff exponent */
        vec2 lspotFactor = blocks[4].zw;

        /* attenuation factor */
        vec3 lattenuation = vec3(blocks[3].w, blocks[4].xy);

        vec3 curSpec = vec3(0.0);
        linear += phong_blinn_single(blocks[0].x, lightDir, eyeDir, ldiffuse, fragNormal, lspotDirection, lspotFactor, 
            lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
        linearSpec += curSpec;
    }

    return linear;
}