#include "samples:shaders/sources/common/version.def"

uniform sampler2D AOMap;
uniform int AOEnabled;

// Get fragment precalculated Ambient Occlusion with PCF filtration 
float getAmbientOcclusion(vec2 uv)
{
    if (AOEnabled == 0)
    {
        return 1.0;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(AOMap, 0));

    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(AOMap, uv + offset).r;
        }
    }

    return result / (4.0 * 4.0);
}
