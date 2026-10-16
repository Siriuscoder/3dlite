#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowTransform[LITE3D_SHADOW_CACHE_MAX_COUNT];
};

#include "samples:shaders/sources/shadow/utils.glsl"
#include "samples:shaders/sources/shadow/pcf.glsl"

// Warning! This method evaluates only the near CSM cascade
float Shadow(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Do not cast shadows
    if (!hasFlag(source.flags, LITE3D_LIGHT_SHADOW_STATIC | LITE3D_LIGHT_SHADOW_DYNAMIC))
        return 1.0;
    if (source.shadowIndex < 0)
        return 1.0;

    int shadowIndex = source.shadowIndex;
    if (hasFlag(source.flags, LITE3D_LIGHT_POINT))
    {
        shadowIndex = source.shadowIndex + cubeFaceFromDir(-angular.lightDir);
    }

    // Shadow space NDC coordinates of current fragment
    vec4 sv = shadowTransform[shadowIndex] * vec4(surface.wv, 1.0);
    // transform the NDC coordinates to the range [0,1]
    vec3 shadowPos = (sv.xyz / sv.w) * 0.5 + 0.5;
    // clipping
    if (shadowPos.z > 1.0 || shadowPos.z < 0.0 || !isValidUV(shadowPos.xy))
        return 0.0;

    // Adaptive bias, filter size
    vec3 adapt = CalcAdaptiveShadowParams(angular, 0);
    return EvaluateShadowSample(shadowPos, adapt, shadowIndex);
}
