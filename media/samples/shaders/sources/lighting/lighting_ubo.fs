//struct ligthInfo 
//{
//    vec4 blocks[6];
//};

// declaration for SSBO
//layout(std430) buffer lightSources 
//{
//    readonly ligthInfo lights[];
//};

//layout(std430) buffer lightIndexes 
//{
//    readonly int indexes[];
//};

// declaration for UBO
layout(std140) uniform lightSources
{
    vec4 lights[6 * 100];
};

layout(std140) uniform lightIndexes
{
    ivec4 indexes[100];
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

    int count = indexes[0].x;
    for(int i = 0; i < count; i++)
    {
        int index = indexes[(i+1)/4][int(mod(i+1, 4))] * 6;
        /* blocks[0].x - type */
        /* blocks[0].y - enabled */
        /* blocks[0].z - affect radius */
        /* blocks[0].w - position.x */
        if (lights[index+0].y == 0)
            continue;

        /* blocks[1].x - position.y */
        /* blocks[1].y - position.z  */
        /* blocks[1].z - ambient.r */
        /* blocks[1].w - ambient.g */
        /* light position */
        vec3 lposition = vec3(lights[index+0].w, lights[index+1].x, lights[index+1].y);
        /* calculate direction from fragment to light */
        vec3 lightDir = lposition - fragPos;
        /* distance to light */
        float dist = length(lightDir);
        /* do not check light distance for directional lights */
        if (lights[index+0].x != 2 && dist > lights[index+0].z)
            continue;

        /* ambient color */
        vec3 lambient = vec3(lights[index+1].z, lights[index+1].w, lights[index+2].x);
        /* diffuse color */
        vec3 ldiffuse = vec3(lights[index+2].y, lights[index+2].z, lights[index+2].w);
        /* specular color */
        vec3 lspecular = vec3(lights[index+3].x, lights[index+3].y, lights[index+3].z);

        /* spot directional */
        /* take effect with spot and directional light */
        vec3 lspotDirection = vec3(lights[index+3].w, lights[index+4].x, lights[index+4].y);
        /* x - spot cutoff */
        /* y - spot cutoff exponent */
        vec2 lspotFactor = vec2(lights[index+4].z, lights[index+4].w);

        /* calculate direction from fragment to eye */
        vec3 eyeDir = normalize(eye - fragPos);
        /* attenuation factor */
        /* blocks[5].x - constant attenuation */
        /* blocks[5].y - linear attenuation */
        /* blocks[5].z - quadratic attenuation */
        vec3 lattenuation = vec3(lights[index+5].x, lights[index+5].y, lights[index+5].z);
        //vec3 lattenuation = vec3(0.5, 0.001, 0.00006);

        vec3 curSpec = vec3(0.0);
#ifdef PHONG_BLINN
        linear += phong_blinn_single(int(lights[index+0].x), lightDir, eyeDir, ldiffuse, lspecular, fragNormal, 
            lspotDirection, lspotFactor, lattenuation, specularFactor, wrapAroundFactor, specPower, curSpec);
#endif
        linearSpec += curSpec;
    }

    return linear;
}