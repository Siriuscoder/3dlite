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
    for (float x = -1.5; x < 1.5; ++x)
    {
        for (float y = -1.5; y < 1.5; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            result += texture(AOMap, uv + offset).r;
        }
    }

    return result / (4.0 * 4.0);
}
