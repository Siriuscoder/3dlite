#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowTransform[LITE3D_SPOT_LITE3D_SHADOW_MAX_COUNT];
};

float Shadow(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Do not cast shadows
    if (!hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW))
        return 1.0;

    // Shadow space NDC coorts of current fragment
    vec4 sv = shadowTransform[source.shadowIndex] * vec4(surface.vw, 1.0);
    // transform the NDC coordinates to the range [0,1]
    sv = (sv / sv.w) * 0.5 + 0.5;
    // Z clip 
    if (sv.z > 1.0 || sv.z < 0.0)
        return 0.0;

    // Adaptive bias
    float bias = max(LITE3D_SHADOW_MAX_ADAPTIVE_BIAS * (1.0 - angular.NdotL), LITE3D_SHADOW_MIN_ADAPTIVE_BIAS);
    if (!isValidUV(sv.xy))
        return 0.0;

    return texture(ShadowMaps, vec4(sv.xy, source.shadowIndex, sv.z - bias));
}
