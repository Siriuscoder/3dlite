#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;
uniform sampler2DArray GBuffer;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowTransform[LITE3D_SPOT_SHADOW_MAX_COUNT];
};

#define LITE3D_POISSON_DISC_COUNT 30
const vec2 PoissonDisc[LITE3D_POISSON_DISC_COUNT] = vec2[](
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

#ifdef LITE3D_SSS_ENABLE
float SSS(vec3 P, vec3 L, float minDepthThreshold)
{
    // Compute ray position and direction (in view-space)
    vec3 rayPos = worldToViewSpacePosition(P);
    vec3 rayDir = worldToViewSpaceDirection(L);
    
    // Compute ray step
    vec3 rayStep = rayDir * LITE3D_SSS_STEP_LENGTH;
    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < LITE3D_SSS_MAX_STEPS; i++)
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

        if (depthDelta > minDepthThreshold && depthDelta < LITE3D_SSS_MAX_DEPTH_THRESHOLD)
        {
            // Mark as occluded
            occlusion = fadeScreenEdge(rayUV);
            break;
        }
    }

    return 1.0 - occlusion;
}
#endif

/* 
    Calculate the adaptive parameters depending the light angle to surface
    x - bias
    y - FilterSize
    z - SSS Depth Threshold
    w - Step
*/
vec4 CalcAdaptiveShadowParams(in AngularInfo angular)
{
    vec3 minV = vec3(LITE3D_SHADOW_MIN_ADAPTIVE_BIAS, LITE3D_SHADOW_MIN_ADAPTIVE_FILTER_SIZE, 0.0);
    vec3 maxV = vec3(LITE3D_SHADOW_MAX_ADAPTIVE_BIAS, LITE3D_SHADOW_MAX_ADAPTIVE_FILTER_SIZE, LITE3D_SSS_MAX_ADAPTIVE_DEPTH_THRESHOLD);
    vec3 rV = max(maxV * (1.0 - angular.NdotL), minV);
    return vec4(rV, max(angular.NdotL, LITE3D_SHADOW_MIN_ADAPTIVE_STEP));
}

float Shadow(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Do not cast shadows
    if (!hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW))
        return 1.0;

    // Shadow space NDC coorts of current fragment
    vec4 sv = shadowTransform[source.shadowIndex] * vec4(surface.wv, 1.0);
    // transform the NDC coordinates to the range [0,1]
    sv = (sv / sv.w) * 0.5 + 0.5;
    // Z clip 
    if (sv.z > 1.0 || sv.z < 0.0)
        return 0.0;

    float shadowFactor = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMaps, 0).xy;
    // Adaptive bias, filter size, step
    vec4 adaptiveParams = CalcAdaptiveShadowParams(angular);
    float samples = 0.0;

    if (hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW_PCF3x3))
    {
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                vec2 shift = sv.xy + (vec2(x, y) * texelSize * adaptiveParams.y);
                if (!isValidUV(shift))
                    continue;

                shadowFactor += texture(ShadowMaps, vec4(shift, source.shadowIndex, sv.z - adaptiveParams.x));
                samples += 1.0;
            }
        }
    }
    else if (hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW_PCF_ADAPTIVE))
    {
        for (float x = -1.5; x <= 1.5; x += adaptiveParams.w)
        {
            for (float y = -1.5; y <= 1.5; y += adaptiveParams.w)
            {
                vec2 shift = sv.xy + (vec2(x, y) * texelSize * adaptiveParams.y);
                if (!isValidUV(shift))
                    continue;

                shadowFactor += texture(ShadowMaps, vec4(shift, source.shadowIndex, sv.z - adaptiveParams.x));
                samples += 1.0;
            }
        }
    }
    else if (hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW_POISSON))
    {
        for (int i = 0; i < LITE3D_POISSON_DISC_COUNT; ++i)
        {
            vec2 shift = sv.xy + (PoissonDisc[i] * texelSize * adaptiveParams.y);
            if (!isValidUV(shift))
                continue;

            shadowFactor += texture(ShadowMaps, vec4(shift, source.shadowIndex, sv.z - adaptiveParams.x));
            samples += 1.0;
        }
    }
    else // Simple shadow without PCF 
    {
        if (!isValidUV(sv.xy))
            return 0.0;

        shadowFactor += texture(ShadowMaps, vec4(sv.xy, source.shadowIndex, sv.z - adaptiveParams.x));
        samples += 1.0;
    }

    if (!isZero(shadowFactor))
    {
        shadowFactor /= samples;

#ifdef LITE3D_SSS_ENABLE
        if (hasFlag(source.flags, LITE3D_LIGHT_CASTSHADOW_SSS))
        {
            shadowFactor *= SSS(surface.wv, angular.lightDir, adaptiveParams.z);
        }
#endif
    }

    return shadowFactor;
}
