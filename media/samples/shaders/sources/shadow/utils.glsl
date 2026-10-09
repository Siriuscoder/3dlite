/* 
    Calculate the adaptive parameters depending the light angle to surface
    x - bias
    y - FilterSize
    z - SSS Depth Threshold
*/
vec3 CalcAdaptiveShadowParams(in AngularInfo angular, int cascade)
{
    float minBias = mix(LITE3D_SHADOW_MIN_BIAS, LITE3D_SHADOW_MAX_BIAS, 
        float(cascade) / LITE3D_SHADOW_CSM_CASCADE_COUNT);  

    vec3 minV = vec3(minBias, LITE3D_SHADOW_PCF_MIN_FILTER_SIZE, 0.0);
    vec3 maxV = vec3(LITE3D_SHADOW_MAX_BIAS, 
        LITE3D_SHADOW_PCF_MAX_FILTER_SIZE, 
        LITE3D_SSS_MAX_ADAPTIVE_DEPTH_THRESHOLD);

    float factor = pow(1.0 - angular.NdotL, 2.3);
    return mix(minV, maxV, factor);
}

float BuildCascadeSplit(int i, float zNear, float zFar)
{
    float p = float(i + 1) / float(LITE3D_SHADOW_CSM_CASCADE_COUNT);
    float logSplit = zNear * pow(zFar / zNear, p);
    float uniformSplit = zNear + (zFar - zNear) * p;

    return mix(uniformSplit, logSplit, LITE3D_SHADOW_CSM_SPLIT_LAMBDA);
}
