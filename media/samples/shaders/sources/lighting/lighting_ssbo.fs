struct ligthInfo 
{
    vec4 blocks[6];
};

layout(std430) buffer lightSources 
{
    readonly ligthInfo lights[];
};

layout(std430) buffer lightIndexes 
{
    readonly int indexes[];
};

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

    int count = indexes[0];
    for (int i = 1; i <= count; i++)
    {
        int index = indexes[i];
        ligthInfo light = lights[index];
        /* blocks[0].x - type */
        /* blocks[0].y - enabled */
        /* blocks[0].z - affect radius */
        /* blocks[0].w - position.x */
        if (int(light.blocks[0].y) == 0)
            continue;

        /* blocks[1].x - position.y */
        /* blocks[1].y - position.z  */
        /* blocks[1].z - ambient.r */
        /* blocks[1].w - ambient.g */
        /* light position */
        vec3 lposition = vec3(light.blocks[0].w, light.blocks[1].x, light.blocks[1].y);
        /* calculate direction from fragment to light */
        vec3 lightDir = lposition - fragPos;
        /* distance to light */
        float dist = length(lightDir);
        /* check light distance */
        if (dist > light.blocks[0].z)
            continue;

        /* ambient color */
        vec3 lambient = vec3(light.blocks[1].z, light.blocks[1].w, light.blocks[2].x);
        /* diffuse color */
        vec3 ldiffuse = vec3(light.blocks[2].y, light.blocks[2].z, light.blocks[2].w);
        /* specular color */
        vec3 lspecular = vec3(light.blocks[3].x, light.blocks[3].y, light.blocks[3].z);

        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = vec3(light.blocks[3].w, light.blocks[4].x, light.blocks[4].y);
        /* x - spot cutoff */
        /* y - spot cutoff exponent */
        vec2 lspotFactor = vec2(light.blocks[4].z, light.blocks[4].w);

        /* calculate direction from fragment to eye */
        vec3 eyeDir = normalize(eye - fragPos);
        /* attenuation factor */
        /* blocks[5].x - constant attenuation */
        /* blocks[5].y - linear attenuation */
        /* blocks[5].z - quadratic attenuation */
        vec3 lattenuation = vec3(light.blocks[5].x, light.blocks[5].y, light.blocks[5].z);

        vec3 curSpec = vec3(0.0);
#ifdef PHONG_BLINN
        linear += phong_blinn_single(int(light.blocks[0].x), lightDir, eyeDir, ldiffuse, lspecular, fragNormal, 
            lspotDirection, lspotFactor, lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
#endif
        linearSpec += curSpec;
    }

    return linear;
}