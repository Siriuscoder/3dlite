#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[SHADOW_MAX_LAYERS];
};

const vec2 PoissonPoints[30] = vec2[](
    vec2(0, 0),
    vec2(1.28766, -0.754559),
    vec2(-1.21724, -0.0903061),
    vec2(0.0237307, -0.300582),
    vec2(-0.763225, -0.989532),
    vec2(1.39046, 1.48874),
    vec2(0.66185, -0.251839),
    vec2(-0.417605, 0.773383),
    vec2(1.0879, -0.369088),
    vec2(0.924448, -0.644191),
    vec2(0.540958, -0.548333),
    vec2(1.39424, 0.0966416),
    vec2(-0.279926, 0.168037),
    vec2(-0.691148, -1.36196),
    vec2(1.03213, 0.219091),
    vec2(-0.06927, -1.38756),
    vec2(0.693741, 1.11407),
    vec2(0.322514, -1.34962),
    vec2(-0.549545, 0.0104367),
    vec2(-1.16988, 1.42763),
    vec2(0.316867, 1.05319),
    vec2(0.0956817, -0.635039),
    vec2(-1.14989, -0.561328),
    vec2(-1.44998, 0.755789),
    vec2(-1.0999, -1.12234),
    vec2(0.580211, -1.06318),
    vec2(-0.0673626, 0.899547),
    vec2(1.04939, 1.32114),
    vec2(1.43786, 1.09785),
    vec2(-0.819775, 0.40949)
);

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

    for (int i = 0; i < 30; ++i)
    {
        vec2 shift = sv.xy + (PoissonPoints[i] * texelSize * SHADOW_MIN_ADAPTIVE_FILTER_SIZE);
        if (!isValidUV(shift))
            continue;

        visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - bias));
    }

    visibility /= 30.0;
    return visibility;
}
