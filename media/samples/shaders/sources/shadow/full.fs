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

float EvaluateShadow(uint flags, int shadowIndex, vec3 adapt, vec3 pos, in AngularInfo angular, inout bool isValid)
{
    isValid = true;
    // Shadow space NDC coordinates of current fragment
    vec4 sv = shadowTransform[shadowIndex] * vec4(pos, 1.0);
    // transform the NDC coordinates to the range [0,1]
    vec3 shadowPos = (sv.xyz / sv.w) * 0.5 + 0.5;
    // clipping
    if (shadowPos.z > 1.0 || shadowPos.z < 0.0 || !isValidUV(shadowPos.xy))
    {
        isValid = false;
        return 1.0;
    }

    if (hasFlag(flags, LITE3D_LIGHT_SHADOW_PCF3x3))
    {
        return EvaluateShadowPCF3x3(shadowPos, adapt, shadowIndex);
    }
    else if (hasFlag(flags, LITE3D_LIGHT_SHADOW_PCF_ADAPTIVE))
    {
        return EvaluateShadowPCFAdaptive(shadowPos, adapt, shadowIndex);
    }
    else if (hasFlag(flags, LITE3D_LIGHT_SHADOW_POISSON))
    {
        return EvaluateShadowPCFPoisson(shadowPos, adapt, shadowIndex);
    }
#ifdef LITE3D_SHADOW_VSM_ENABLE
    else if (hasFlag(flags, LITE3D_LIGHT_SHADOW_VSM))
    {
        return EvaluateShadowEVSM2(shadowPos, shadowIndex);
    }
#endif
    else // Simple shadow without PCF 
    {
        return EvaluateShadowSample(shadowPos, adapt, shadowIndex);
    }

    isValid = false;
    return 1.0;
}

float Shadow(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Do not cast shadows
    if (!hasFlag(source.flags, LITE3D_LIGHT_SHADOW_STATIC | LITE3D_LIGHT_SHADOW_DYNAMIC))
        return 1.0;
    if (source.shadowIndex < 0)
        return 1.0;

    int shadowIndex = source.shadowIndex;
    float shadowVisibility = 0.0;
    vec3 adapt = vec3(0.0);
    bool isValidShadow = true;

    if (hasFlag(source.flags, LITE3D_LIGHT_POINT))
    {
        shadowIndex = source.shadowIndex + cubeFaceFromDir(-angular.lightDir);
    }
#ifdef LITE3D_SHADOW_CSM_ENABLE
    if (hasFlag(source.flags, LITE3D_LIGHT_DIRECTIONAL))
    {
        int cascade = LITE3D_SHADOW_CSM_CASCADE_COUNT - 1;
        vec3 viewPos = worldToViewSpacePosition(surface.wv);
        float zNear = getZNear();
        float zFar = getZFar();
        float depth = -viewPos.z;
        float splitEnd = 0.0;

        for (int i = 0; i < LITE3D_SHADOW_CSM_CASCADE_COUNT - 1; ++i)
        {
            splitEnd = BuildCascadeSplit(i, zNear, zFar);
            if (depth < splitEnd)
            {
                cascade = i;
                break;
            }
        }

        shadowIndex = source.shadowIndex + cascade + 1;
        adapt = CalcAdaptiveShadowParams(angular, cascade);
        shadowVisibility = EvaluateShadow(source.flags, shadowIndex, adapt, surface.wv, angular, isValidShadow);

        // Blend with neighbor cascade, if current is not last
        if (cascade < LITE3D_SHADOW_CSM_CASCADE_COUNT - 1)
        {
            const float blendThreshold = 0.1;

            float splitStart = cascade == 0 ? zNear : BuildCascadeSplit(cascade - 1, zNear, zFar);
            float splitSize = splitEnd - splitStart;
            float fadeFactor = (splitEnd - depth) / splitSize;

            if (fadeFactor < blendThreshold)
            {
                vec3 adaptNext = CalcAdaptiveShadowParams(angular, cascade + 1);
                float nextCascadeVisibility = EvaluateShadow(source.flags, shadowIndex + 1, adaptNext, surface.wv, angular, isValidShadow);

                if (isValidShadow)
                {
                    float weight = smoothstep(0.0, blendThreshold, fadeFactor);
                    shadowVisibility = mix(nextCascadeVisibility, shadowVisibility, weight);
                }
            }
        }
    }
    else
#endif
    {
        // Adaptive bias, filter size
        adapt = CalcAdaptiveShadowParams(angular, 0);
        shadowVisibility = EvaluateShadow(source.flags, shadowIndex, adapt, surface.wv, angular, isValidShadow);
    }

    if (!isZero(shadowVisibility))
    {
#ifdef LITE3D_SSS_ENABLE
        if (hasFlag(source.flags, LITE3D_LIGHT_SHADOW_SSS))
        {
            shadowVisibility = min(shadowVisibility, SSS(surface.wv, angular.lightDir, adapt.z));
        }
#endif
    }

    return shadowVisibility;
}
