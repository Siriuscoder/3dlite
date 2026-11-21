uniform sampler2D AOMap;
uniform int AOEnabled;

vec4 resolveAO(ivec2 pixelCoord, vec3 geoNormal, vec3 worldCoord)
{
    if (AOEnabled == 0)
    {
        return vec4(geoNormal, 1.0);
    }

    ivec2 halfSize = textureSize(AOMap, 0);
    // Координаты в half-resolution текстуре
    ivec2 base = ivec2(floor((vec2(pixelCoord) + 0.5) * 0.5 - 0.5));
    // Линейная глубина текущего пикселя
    float depth = -worldToView(worldCoord).z;

    float minDiff = 1e20;
    vec2 bestNormal = vec2(0.0, 0.0);
    float visibility = 0.0;

    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            ivec2 sampleCoord = clamp(base + ivec2(x, y), ivec2(0), halfSize - 1);
            vec4 sampleData = texelFetch(AOMap, sampleCoord, 0);

            float diff = abs(sampleData.w - depth);

            if (diff < minDiff)
            {
                minDiff = diff;
                bestNormal = sampleData.xy;
            }

            visibility += sampleData.z;
        }
    }

    visibility /= 4.0;

    return vec4(unpackNormal(bestNormal), visibility);
}
