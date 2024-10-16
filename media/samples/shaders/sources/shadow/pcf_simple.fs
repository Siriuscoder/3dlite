#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[SHADOW_MAX_LAYERS];
};

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

    // Adaptive bias
    float bias = max(SHADOW_MAX_ADAPTIVE_BIAS * (1.0 - dot(N, L)), SHADOW_MIN_ADAPTIVE_BIAS);
    if (!isValidUV(sv.xy))
        return 0.0;

    return texture(ShadowMaps, vec4(sv.xy, shadowIndex, sv.z - bias));
}
