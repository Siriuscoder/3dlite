#include "samples:shaders/sources/common/version.def"

uniform sampler2D AOMap;

// Get fragment precalculated Ambient Occlusion with PCF filtration 
float GetAO(vec2 iuv)
{
    vec2 texelSize = 1.0 / vec2(textureSize(AOMap, 0));

    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(AOMap, iuv + offset).r;
        }
    }

    return result / (4.0 * 4.0);
}
