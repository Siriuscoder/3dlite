#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[SHADOW_MAX_LAYERS];
};

/* 
    Calculate the adaptive parameters depending the light angle to surface 
    x - bias
    y - FilterSize
    z - Step
*/
vec3 CalcAdaptiveShadowParams(vec3 N, vec3 L)
{
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    vec2 minV = vec2(SHADOW_MIN_ADAPTIVE_BIAS, SHADOW_MIN_ADAPTIVE_FILTER_SIZE);
    vec2 maxV = vec2(SHADOW_MAX_ADAPTIVE_BIAS, SHADOW_MAX_ADAPTIVE_FILTER_SIZE);
    vec2 rV = max(maxV * (1.0 - NdotL), minV);
    return vec3(rV, max(NdotL, SHADOW_MIN_ADAPTIVE_STEP));
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
    vec3 params = CalcAdaptiveShadowParams(N, L);
    float samples = 0.0;

    for (float x = -1.5; x <= 1.5; x += params.z)
    {
        for (float y = -1.5; y <= 1.5; y += params.z)
        {
            vec2 shift = sv.xy + (vec2(x, y) * texelSize * params.y);
            if (!isValidUV(shift))
                continue;

            visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - params.x));
            samples += 1.0;
        }
    }

    if (!fiszero(visibility))
    {
        visibility /= samples;
    }

    return visibility;
}
