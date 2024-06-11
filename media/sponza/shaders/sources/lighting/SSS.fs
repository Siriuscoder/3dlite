#include "sponza:shaders/sources/inc/common.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;
uniform sampler2DArray GBuffer;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[MAX_SHADOW_LAYERS];
};

const float shadowBiasMax       = 0.0028;
const float shadowBiasMin       = 0.0008;
const float shadowFilterSize    = 1.12;
const int   sssMaxSteps         = 16;     // Max ray steps, affects quality and performance.
const float sssMaxRayDistance   = 0.55;   // Max shadow length, longer shadows are less accurate.
const float sssDepthThickness   = 0.055;   // Depth testing thickness.
const float sssStepLength       = sssMaxRayDistance / float(sssMaxSteps);

float SSS(vec3 vw, vec3 L)
{
    // Compute ray position and direction (in view-space)
    vec3 ray_pos = worldToViewSpacePosition(vw);
    vec3 ray_dir = worldToViewSpaceDirection(L);

    // Compute ray step
    vec3 ray_step = ray_dir * sssStepLength;

    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < sssMaxSteps; i++)
    {
        // Step the ray
        ray_pos += ray_step;
        vec2 ray_uv = viewPositionToUV(ray_pos);

        // Ensure the UV coordinates are inside the screen
        if (!isValidUV(ray_uv))
            return 1.0;
        
        // Compute the difference between the ray's and the camera's depth
        float depth_z = worldToViewSpacePosition(texture(GBuffer, vec3(ray_uv, 0)).xyz).z;
        float depth_delta = depth_z - ray_pos.z;

        if (depth_delta > 0.0 && depth_delta < sssDepthThickness)
        {
            // Mark as occluded
            occlusion = fadeScreenEdge(ray_uv);
            break;
        }
    }

    return 1.0 - occlusion;
}

float PCF(float shadowIndex, vec3 vw, vec3 N, vec3 L)
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
