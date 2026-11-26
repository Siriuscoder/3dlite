uniform sampler2D AOMap;
uniform int AOEnabled;

vec4 resolveBent(ivec2 pixelCoord, vec3 geoNormal, vec3 worldCoord)
{
    if (AOEnabled == 0)
    {
        return vec4(geoNormal, 1.0);
    }

    ivec2 halfSize = textureSize(AOMap, 0);
    // Координаты в half-resolution текстуре
    vec2 halfPos = (vec2(pixelCoord) + 0.5) * 0.5 - 0.5;
    ivec2 base = ivec2(floor(halfPos));
    // Линейная глубина текущего пикселя
    float currentDepth = -worldToView(worldCoord).z;

    vec2 f = fract(halfPos);

    float minDiff = 1e20;
    vec3 bentSum = vec3(0.0);
    float bestAO = 1.0;
    float weightSum = 0.0; 

    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            ivec2 offset = ivec2(x, y);
            ivec2 sampleCoord = clamp(base + offset, ivec2(0), halfSize - 1);
            vec4 sampleData = texelFetch(AOMap, sampleCoord, 0);

            vec3 sampleBent = unpackNormal(sampleData.xy);
            float sampleAO = sampleData.z;
            float sampleDepth = sampleData.w;

            // Вес по глубине
            float depthDiff = abs(sampleDepth - currentDepth);
            float depthDiffRel = depthDiff / max(abs(currentDepth), 1e-6);
            float depthWeight = exp(-depthDiffRel * 40.0);

            // Билинейный вес
            vec2 d = abs(f - vec2(offset));
            float spatialWeight = (1.0 - d.x) * (1.0 - d.y);

            float weight = spatialWeight * depthWeight;

            bentSum += sampleBent * weight;
            weightSum += weight;

            if (depthDiff < minDiff)
            {
                minDiff = depthDiff;
                bestAO = sampleAO;
            }
        }
    }

    if (weightSum > 1e-6)
    {
        bentSum /= weightSum;
        // Оценка сколько занимает пиксель AO текстуры по отношению к радиусу AO.
        vec2 gtaoPixelSize = pixelWorldSize(currentDepth, halfSize);
        float gtaoRadiusPixels = LITE3D_GTAO_RADIUS / max(gtaoPixelSize.x, gtaoPixelSize.y);

        // Чем меньше затенение занимает пикселей на экране, тем грубее будет bentNormal, поэтому плавно подменяем ее на geoNormal
        float bentWeight = smoothstep(6.0, 50.0, gtaoRadiusPixels);
        bentSum = normalize(mix(geoNormal, bentSum, bentWeight));
    }
    else
    {
        bentSum = geoNormal;
    }

    return vec4(bentSum, bestAO);
}
