#include "samples:shaders/sources/common/common_inc.glsl"

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

layout(std140) uniform LightSources
{
    LightSource lights[LITE3D_MAX_LIGHT_COUNT];
};

layout(std140) uniform LightIndexes
{
    ivec4 lightsIndexes[LITE3D_MAX_LIGHT_COUNT];
};

#include "samples:shaders/sources/pbr/direct.glsl"

/* Indirect lighting */
SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, vec3(0.0), vec3(0.0));
    AngularInfo angular;
    angularInfoInit(angular, surface);

    SufraceLighting direct = SufraceLighting(vec3(0.0), vec3(0.0), vec3(0.0));

    int count = lightsIndexes[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = lightsIndexes[i / 4][i % 4];
        SufraceLighting sl = EvaluateDirectLight(lights[index], surface, angular);

        direct.diffuse += sl.diffuse;
    }

    // Calculate indirect lighting, ambient, IBL .. 
    SufraceLighting indirect = EvaluateIBL(surface, angular);
    // Total sum
    vec3 total =  direct.diffuse + indirect.diffuse + surface.material.emission.rgb;
    // Compute total illumination 
    fragColor = vec4(total, surface.material.alpha);
}
