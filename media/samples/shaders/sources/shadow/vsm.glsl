uniform sampler2DArray MomentsMaps;

// Reduces VSM light bleeding
float ReduceLightBleeding(float pMax, float amount)
{
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, pMax);
}

float ChebyshevUpperBound(
    vec2 moments,
    float mean)
{
    // Compute variance
    float variance = moments.y - moments.x * moments.x;
    variance = max(variance, LITE3D_SHADOW_VSM_MIN_VARIANCE);

    // Compute probabilistic upper bound
    float d = mean - moments.x;
    float pMax = variance / (variance + d * d);

    pMax = ReduceLightBleeding(pMax, LITE3D_SHADOW_VSM_LIGHT_BLEEDING_REDUCTION);

    // One-tailed Chebyshev
    return mean <= moments.x ? 1.0 : pMax;
}

float EvaluateShadowVSM(vec3 shadowPos, vec3 adaptive, int shadowIndex)
{
    float sampleDepth = shadowPos.z - (adaptive.x * 0.01);
    vec2 occluder = texture(MomentsMaps, vec3(shadowPos.xy, shadowIndex)).xy;

    return ChebyshevUpperBound(occluder, sampleDepth);
}
