#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;
uniform sampler2DArray GBuffer;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[MAX_SHADOW_LAYERS];
};

float SSS(vec3 vw, vec3 L)
{
    // Compute ray position and direction (in view-space)
    vec3 rayPos = worldToViewSpacePosition(vw);
    vec3 rayDir = worldToViewSpaceDirection(L);

    // Compute ray step
    vec3 rayStep = rayDir * sssStepLength;
    float depthOriginal = rayPos.z;

    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < sssMaxSteps; i++)
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

        if (depthDelta > 0.0 && depthDelta < sssDepthThickness && abs(depthOriginal - rayPos.z) < sssMaxDepthVariance)
        {
            // Mark as occluded
            occlusion = fadeScreenEdge(rayUV);
            break;
        }
    }

    return 1.0 - occlusion;
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
    // Adaptive bias
    float bias = max(shadowBiasMax * (1.0 - dot(N, L)), shadowBiasMin);

    for (float x = -1.5; x <= 1.5; x += 1.0)
    {
        for (float y = -1.5; y <= 1.5; y += 1.0)
        {
            vec2 shift = sv.xy + (vec2(x, y) * texelSize * shadowFilterSize);
            if (!isValidUV(shift))
                continue;

            visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - bias));
        }
    }

    visibility /= 16.0;
    if (!fiszero(visibility))
    {
        visibility *= SSS(vw, L);
    }

    return visibility;
}
