float EvaluateShadowPCF3x3(vec3 shadowPos, vec3 adaptive, int shadowIndex)
{
    float sampleDepth = shadowPos.z - adaptive.x; // Biasing
    vec2 kernelSize = adaptive.y / textureSize(ShadowMaps, 0).xy;
    float shadowFactor = 0.0;
    float samples = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 shift = shadowPos.xy + (vec2(x, y) * kernelSize);
            if (!isValidUV(shift))
                continue;

            shadowFactor += texture(ShadowMaps, vec4(shift, shadowIndex, sampleDepth));
            samples += 1.0;
        }
    }

    return shadowFactor / max(samples, FLT_EPSILON);
}

float EvaluateShadowPCFAdaptive(vec3 shadowPos, vec3 adaptive, int shadowIndex)
{
    float sampleDepth = shadowPos.z - adaptive.x; // Biasing
    vec2 kernelSize = adaptive.y / textureSize(ShadowMaps, 0).xy;
    float shadowFactor = 0.0;
    float samples = 0.0;

    for (float x = -1.5; x <= 1.5; x += LITE3D_SHADOW_PCF_MIN_STEP)
    {
        for (float y = -1.5; y <= 1.5; y += LITE3D_SHADOW_PCF_MIN_STEP)
        {
            vec2 shift = shadowPos.xy + (vec2(x, y) * kernelSize);
            if (!isValidUV(shift))
                continue;

            shadowFactor += texture(ShadowMaps, vec4(shift, shadowIndex, sampleDepth));
            samples += 1.0;
        }
    }

    return shadowFactor / max(samples, FLT_EPSILON);
}

float EvaluateShadowPCFPoisson(vec3 shadowPos, vec3 adaptive, int shadowIndex)
{
    float sampleDepth = shadowPos.z - adaptive.x; // Biasing
    vec2 kernelSize = adaptive.y / textureSize(ShadowMaps, 0).xy;
    float shadowFactor = 0.0;
    float samples = 0.0;

    for (int i = 0; i < 30; ++i)
    {
        vec2 shift = shadowPos.xy + (PoissonDisk(i) * kernelSize);
        if (!isValidUV(shift))
            continue;

        shadowFactor += texture(ShadowMaps, vec4(shift, shadowIndex, sampleDepth));
        samples += 1.0;
    }

    return shadowFactor / max(samples, FLT_EPSILON);
}

float EvaluateShadowSample(vec3 shadowPos, vec3 adaptive, int shadowIndex)
{
    float sampleDepth = shadowPos.z - adaptive.x; // Biasing
    return texture(ShadowMaps, vec4(shadowPos.xy, shadowIndex, sampleDepth));
}
