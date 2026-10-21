uniform sampler2DArray MomentsMaps;


float GetEVSMExponent()
{
    const float maxExponent = 42.0;
    // Clamp to maximum range of fp32 to prevent overflow/underflow
    return clamp(LITE3D_VSM_EXPONENT, 1.0, maxExponent);
}

float WarpDepth(float depth, float exponent)
{
    // Rescale depth into [-1, 1]
    depth = 2.0 * depth - 1.0;
    return exp(exponent * depth);
}

// Reduces VSM light bleeding
float ReduceLightBleeding(float pMax, float amount)
{
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, pMax);
}

float ChebyshevUpperBound(vec2 moments, float mean, float minVariance)
{
    // Compute variance
    float variance = moments.y - moments.x * moments.x;
    variance = max(variance, minVariance);

    // Compute probabilistic upper bound
    float d = mean - moments.x;
    float pMax = variance / (variance + d * d);

    pMax = ReduceLightBleeding(pMax, LITE3D_SHADOW_VSM_LIGHT_BLEEDING_REDUCTION);

    // One-tailed Chebyshev
    return mean <= moments.x ? 1.0 : pMax;
}

float EvaluateShadowVSM(vec3 shadowPos, int shadowIndex)
{
    vec2 occluder = texture(MomentsMaps, vec3(shadowPos.xy, shadowIndex)).xy;

    return ChebyshevUpperBound(occluder, shadowPos.z, LITE3D_SHADOW_VSM_BIAS);
}

float EvaluateShadowEVSM2(vec3 shadowPos, int shadowIndex)
{
    float exponent = GetEVSMExponent();
    float warpedDepth = WarpDepth(shadowPos.z, exponent);

    vec2 occluder = texture(MomentsMaps, vec3(shadowPos.xy, shadowIndex)).xy;

    // Derivative of warping at depth
    float depthScale = LITE3D_SHADOW_VSM_BIAS * exponent * warpedDepth;
    float minVariance = depthScale * depthScale;

    // Positive only
    return ChebyshevUpperBound(occluder, warpedDepth, minVariance);
}
