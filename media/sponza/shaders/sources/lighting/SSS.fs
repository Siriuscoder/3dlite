#include "sponza:shaders/sources/inc/common.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;
uniform sampler2DArray GBuffer;
uniform mat4 CameraView;
uniform mat4 CameraProjection;
uniform float RandomSeed; /* 0.0 - 1.0 */

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[MAX_SHADOW_LAYERS];
};

in vec2 iuv;

const float shadowBiasMax = 0.0028;
const float shadowBiasMin = 0.0008;

const float pcf_filter_size                   = 1.12;
const int   sss_max_steps                     = 32;     // Max ray steps, affects quality and performance.
const float sss_ray_max_distance              = 0.95;   // Max shadow length, longer shadows are less accurate.
const float sss_thickness                     = 0.1;  // Depth testing thickness.
const float sss_step_length                   = sss_ray_max_distance / float(sss_max_steps);

vec3 viewSpacePosition(vec3 vw)
{
    vec4 pos = CameraView * vec4(vw, 1.0);
    return pos.xyz / pos.w;
}

vec3 viewSpaceDirection(vec3 dirw)
{
    vec4 dir = CameraView * vec4(dirw, 0.0);
    return normalize(dir.xyz);
}

vec3 sampleViewSpacePosition(vec2 uv)
{
    return viewSpacePosition(texture(GBuffer, vec3(uv, 0)).xyz);
}

float fadeScreenEdge(vec2 uv)
{
    vec2 fade = max(vec2(0.0), 12.0 * abs(uv - 0.5) - 5.0);
    return clamp(1.0 - dot(fade, fade), 0.0, 1.0);
}

vec2 viewPosToUV(vec3 pos)
{
    vec4 uv = CameraProjection * vec4(pos, 1.0);
    uv /= uv.w;                 // perspective divide
    return uv.xy * 0.5 + 0.5;   // transform to range 0.0 - 1.0 
}

bool isValidUV(vec2 uv)
{
    return !(uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0);
}
/*
float linearizeDepth(vec2 uv)
{
    float near = 0.1;
    float far  = 100.0;
    float z_b  = texture(Depth, uv).x;
    float z_n  = 2.0 * z_b - 1.0;
    return 2.0 * far * near / (near + far - z_n * (near - far));
} */

float SSS(vec3 vw, vec3 ldir)
{
    // Compute ray position and direction (in view-space)
    vec3 ray_pos = viewSpacePosition(vw);
    vec3 ray_dir = viewSpaceDirection(ldir);

    // Compute ray step
    vec3 ray_step = ray_dir * sss_step_length;

    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < sss_max_steps; i++)
    {
        // Step the ray
        ray_pos += ray_step;
        vec2 ray_uv = viewPosToUV(ray_pos);

        // Ensure the UV coordinates are inside the screen
        if (!isValidUV(ray_uv))
            return 1.0;
        
        // Compute the difference between the ray's and the camera's depth
        float depth_z = sampleViewSpacePosition(ray_uv).z;
        float depth_delta = depth_z - ray_pos.z;

        if (depth_delta > 0.0 && depth_delta < sss_thickness)
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
            vec2 shift = sv.xy + (vec2(x, y) * texelSize * pcf_filter_size);
            if (!isValidUV(shift))
                continue;

            visibility += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - bias));
        }
    }

    return visibility * SSS(vw, L) / 16.0;
}
