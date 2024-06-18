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

    float visibility = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMaps, 0).xy;
    // Adaptive bias
    float bias = max(SHADOW_MAX_ADAPTIVE_BIAS * (1.0 - dot(N, L)), SHADOW_MIN_ADAPTIVE_BIAS);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 shift = sv.xy + (vec2(x, y) * texelSize * SHADOW_MIN_ADAPTIVE_FILTER_SIZE);
            if (!isValidUV(shift))
                continue;

            visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - bias));
        }
    }

    visibility /= 9.0;
    return visibility;
}
