#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowTransform[LITE3D_SHADOW_CACHE_MAX_COUNT];
};

#include "samples:shaders/sources/shadow/utils.glsl"
#include "samples:shaders/sources/shadow/pcf.glsl"

#ifdef LITE3D_SSS_ENABLE
#include "samples:shaders/sources/shadow/sss.glsl"
#endif

#ifdef LITE3D_SHADOW_VSM_ENABLE
#include "samples:shaders/sources/shadow/vsm.glsl"
#endif

float Shadow(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Do not cast shadows
    if (!hasFlag(source.flags, LITE3D_LIGHT_SHADOW_STATIC | LITE3D_LIGHT_SHADOW_DYNAMIC))
        return 1.0;
    if (source.shadowIndex < 0)
        return 1.0;

    int shadowIndex = source.shadowIndex;
    int cascade = 0;
    if (hasFlag(source.flags, LITE3D_LIGHT_POINT))
    {
        shadowIndex = source.shadowIndex + cubeFaceFromDir(-angular.lightDir);
    }
#ifdef LITE3D_SHADOW_CSM_ENABLE
    else if (hasFlag(source.flags, LITE3D_LIGHT_DIRECTIONAL))
    {
        cascade = LITE3D_SHADOW_CSM_CASCADE_COUNT - 1;
        vec3 viewPos = worldToViewSpacePosition(surface.wv);
        float zNear = getZNear();
        float zFar = getZFar();
        float depth = -viewPos.z;

        for (int i = 0; i < LITE3D_SHADOW_CSM_CASCADE_COUNT - 1; ++i)
        {
            if (depth < BuildCascadeSplit(i, zNear, zFar))
            {
                cascade = i;
                break;
            }
        }

        shadowIndex = source.shadowIndex + cascade;
    }
#endif

    // Shadow space NDC coordinates of current fragment
    vec4 sv = shadowTransform[shadowIndex] * vec4(surface.wv, 1.0);
    // transform the NDC coordinates to the range [0,1]
    vec3 shadowPos = (sv.xyz / sv.w) * 0.5 + 0.5;
    // clipping
    if (shadowPos.z > 1.0 || shadowPos.z < 0.0 || !isValidUV(shadowPos.xy))
        return 0.0;

    float shadowFactor = 0.0;
    // Adaptive bias, filter size
    vec3 adapt = CalcAdaptiveShadowParams(angular, cascade);

    if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_PCF3x3))
    {
        shadowFactor = EvaluateShadowPCF3x3(shadowPos, adapt, shadowIndex);
    }
    else if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_PCF_ADAPTIVE))
    {
        shadowFactor = EvaluateShadowPCFAdaptive(shadowPos, adapt, shadowIndex);
    }
    else if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_POISSON))
    {
        shadowFactor = EvaluateShadowPCFPoisson(shadowPos, adapt, shadowIndex);
    }
#ifdef LITE3D_SHADOW_VSM_ENABLE
    else if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_VSM))
    {
        shadowFactor = EvaluateShadowVSM(shadowPos, adapt, shadowIndex);
    }
#endif
    else // Simple shadow without PCF 
    {
        shadowFactor = EvaluateShadowSample(shadowPos, adapt, shadowIndex);
    }

    if (!isZero(shadowFactor))
    {
#ifdef LITE3D_SSS_ENABLE
        if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_SSS))
        {
            shadowFactor = min(shadowFactor, SSS(surface.wv, angular.lightDir, adapt.z));
        }
#endif
    }

    return shadowFactor;
}
