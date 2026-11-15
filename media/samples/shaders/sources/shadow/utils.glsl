/* 
    Calculate the adaptive parameters depending the light angle to surface
    x - bias
    y - FilterSize
    z - SSS Depth Threshold
*/
vec2 CalcAdaptiveShadowParams(in AngularInfo angular, int cascade)
{
    float minBias = mix(LITE3D_SHADOW_MIN_BIAS, LITE3D_SHADOW_MAX_BIAS, 
        float(cascade) / LITE3D_SHADOW_CSM_CASCADE_COUNT);  

    vec2 minV = vec2(minBias, LITE3D_SHADOW_PCF_MIN_FILTER_SIZE);
    vec2 maxV = vec2(LITE3D_SHADOW_MAX_BIAS, LITE3D_SHADOW_PCF_MAX_FILTER_SIZE);

    float factor = fastPow(1.0 - angular.NdotL, 2.3);
    return mix(minV, maxV, factor);
}

float BuildCascadeSplit(int i, float zNear, float zFar)
{
    float p = float(i + 1) / float(LITE3D_SHADOW_CSM_CASCADE_COUNT);
    float logSplit = zNear * pow(zFar / zNear, p);
    float uniformSplit = zNear + (zFar - zNear) * p;

    return mix(uniformSplit, logSplit, LITE3D_SHADOW_CSM_SPLIT_LAMBDA);
}
