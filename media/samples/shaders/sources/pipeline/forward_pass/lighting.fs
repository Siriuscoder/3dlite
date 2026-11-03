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

vec3 ComputeLighting(in Surface surface, in AngularInfo angular)
{
    SufraceLighting direct = SufraceLighting(vec3(0.0), vec3(0.0));

    int count = lightsIndexes[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = lightsIndexes[i/4][int(mod(i, 4))];
        SufraceLighting sl = EvaluateDirect(lights[index], surface, angular);

        direct.diffuse += sl.diffuse;
        direct.specular += sl.specular;
    }

    // Calculate indirect lighting, ambient, IBL .. 
    SufraceLighting indirect = EvaluateIBL(surface, angular);
    // Total sum
    return direct.diffuse + direct.specular + indirect.diffuse + indirect.specular + surface.material.emission.rgb;
}
