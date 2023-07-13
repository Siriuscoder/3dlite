uniform samplerBuffer lightSources;
uniform isamplerBuffer lightIndexes;

#ifdef PHONG_BLINN
vec3 phong_blinn_single(int type, vec3 lightDir, vec3 eyeDir, vec3 diffuse, 
    vec3 specular, vec3 normal, vec3 spotDirection, vec2 spotFactor, vec3 attenuation, 
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec);
#endif

vec3 calc_lighting(vec3 fragPos, 
    vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 linear = vec3(0.0);
    linearSpec = vec3(0.0);
    vec4 blocks[6];

    int count = texelFetch(lightIndexes, 0).r;
    for(int i = 0; i < count; i++)
    {
        int texel = texelFetch(lightIndexes, i+1).r * 6;

        /* blocks[0].x - type */
        /* blocks[0].y - enabled */
        /* blocks[0].z - affect radius */
        /* blocks[0].w - position.x */
        blocks[0] = texelFetch(lightSources, texel + 0);
        if (blocks[0].y == 0)
            continue;

        /* blocks[1].x - position.y */
        /* blocks[1].y - position.z  */
        /* blocks[1].z - ambient.r */
        /* blocks[1].w - ambient.g */
        blocks[1] = texelFetch(lightSources, texel + 1);


        /* light position */
        vec3 lposition = vec3(blocks[0].w, blocks[1].x, blocks[1].y);
        /* calculate direction from fragment to light */
        vec3 lightDir = lposition - fragPos;
        /* distance to light */
        float dist = length(lightDir);
        /* check light distance */
        if (dist > blocks[0].z)
            continue;

        blocks[2] = texelFetch(lightSources, texel + 2);
        blocks[3] = texelFetch(lightSources, texel + 3);
        blocks[4] = texelFetch(lightSources, texel + 4);
        blocks[5] = texelFetch(lightSources, texel + 5);
        /* ambient color */
        vec3 lambient = vec3(blocks[1].z, blocks[1].w, blocks[2].x);
        /* diffuse color */
        vec3 ldiffuse = vec3(blocks[2].y, blocks[2].z, blocks[2].w);
        /* specular color */
        vec3 lspecular = vec3(blocks[3].x, blocks[3].y, blocks[3].z);

        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = vec3(blocks[3].w, blocks[4].x, blocks[4].y);
        /* x - spot cutoff */
        /* y - spot cutoff exponent */
        vec2 lspotFactor = vec2(blocks[4].z, blocks[4].w);

        /* calculate direction from fragment to eye */
        vec3 eyeDir = normalize(eye - fragPos);
        /* attenuation factor */
        /* blocks[5].x - constant attenuation */
        /* blocks[5].y - linear attenuation */
        /* blocks[5].z - quadratic attenuation */
        vec3 lattenuation = vec3(blocks[5].x, blocks[5].y, blocks[5].z);
        //vec3 lattenuation = vec3(0.5, 0.001, 0.00006);

        vec3 curSpec = vec3(0.0);
#ifdef PHONG_BLINN
        linear += phong_blinn_single(int(blocks[0].x), lightDir, eyeDir, ldiffuse, lspecular, fragNormal, 
            lspotDirection, lspotFactor, lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
#endif
        linearSpec += curSpec;
    }

    return linear;
}