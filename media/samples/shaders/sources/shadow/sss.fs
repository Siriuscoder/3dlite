#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;
uniform sampler2DArray GBuffer;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[LITE3D_SPOT_SHADOW_MAX_COUNT];
};

float SSS(vec3 vw, vec3 L, float minDepthThreshold)
{
    // Compute ray position and direction (in view-space)
    vec3 rayPos = worldToViewSpacePosition(vw);
    vec3 rayDir = worldToViewSpaceDirection(L);
    
    // Compute ray step
    vec3 rayStep = rayDir * SSS_STEP_LENGTH;
    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < SSS_MAX_STEPS; i++)
    {
        // Step the ray
        rayPos += rayStep;
        vec2 rayUV = viewPositionToUV(rayPos);

        // Ensure the UV coordinates are inside the screen
        if (!isValidUV(rayUV))
            return 1.0;
        
        // Compute the difference between the ray's and the camera's depth
        float depth = worldToViewSpacePosition(texture(GBuffer, vec3(rayUV, 0)).xyz).z;
        float depthDelta = depth - rayPos.z;

        if (depthDelta > minDepthThreshold && depthDelta < SSS_MAX_DEPTH_THRESHOLD)
        {
            // Mark as occluded
            occlusion = fadeScreenEdge(rayUV);
            break;
        }
    }

    return 1.0 - occlusion;
}

/* 
    Calculate the adaptive parameters depending the light angle to surface
    x - bias
    y - FilterSize
    z - SSS Depth Threshold
    w - Step
*/
vec4 CalcAdaptiveShadowParams(vec3 N, vec3 L)
{
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    vec3 minV = vec3(SHADOW_MIN_ADAPTIVE_BIAS, SHADOW_MIN_ADAPTIVE_FILTER_SIZE, 0.0);
    vec3 maxV = vec3(SHADOW_MAX_ADAPTIVE_BIAS, SHADOW_MAX_ADAPTIVE_FILTER_SIZE, SSS_MAX_ADAPTIVE_DEPTH_THRESHOLD);
    vec3 rV = max(maxV * (1.0 - NdotL), minV);
    return vec4(rV, max(NdotL, SHADOW_MIN_ADAPTIVE_STEP));
}

float ShadowVisibility(float shadowIndex, vec3 vw, vec3 N, vec3 L)
{
    // Do not cast shadows
    if (shadowIndex < 0.0)
        return 1.0;

    // Shadow space NDC coorts of current fragment
    vec4 sv = shadowMat[int(shadowIndex)] * vec4(vw, 1.0);
    // transform the NDC coordinates to the range [0,1]
    sv = (sv / sv.w) * 0.5 + 0.5;
    // Z clip 
    if (sv.z > 1.0 || sv.z < 0.0)
        return 0.0;

    float visibility = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMaps, 0).xy;
    // Adaptive bias, filter size, step
    vec4 params = CalcAdaptiveShadowParams(N, L);
    float samples = 0.0;

    for (float x = -1.5; x <= 1.5; x += params.w)
    {
        for (float y = -1.5; y <= 1.5; y += params.w)
        {
            vec2 shift = sv.xy + (vec2(x, y) * texelSize * params.y);
            if (!isValidUV(shift))
                continue;

            visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - params.x));
            samples += 1.0;
        }
    }

    if (!isZero(visibility))
    {
        visibility /= samples;
        visibility *= SSS(vw, L, params.z);
    }

    return visibility;
}
