#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArray GBuffer;
uniform float AORadius;

in vec2 iuv;
out vec4 outColor;

#ifndef LITE3D_SSAO_MAX_DEPTH_SAMPLES
#define LITE3D_SSAO_MAX_DEPTH_SAMPLES 80
#endif

#ifndef LITE3D_SSAO_SAMPLE_BIAS
#define LITE3D_SSAO_SAMPLE_BIAS 0.030
#endif

#ifndef LITE3D_SSAO_POWER
#define LITE3D_SSAO_POWER 0.45
#endif

vec3 getViewSpacePosition(vec2 uv)
{
    return worldToViewSpacePosition(texture(GBuffer, vec3(uv, 0)).xyz);
}

vec3 getViewSpaceNormal(vec2 uv)
{
    vec3 nw = texture(GBuffer, vec3(uv, 1)).xyz;
    // Non shaded fragment
    if (isZero(nw))
        discard;

    return worldToViewSpaceDirection(nw);
}

void main()
{
    float aoFactor = 0.0;
    float rc = 1.0; // For more randomization
    // sampling fragment normal in view space
    vec3 nv = getViewSpaceNormal(iuv);
    // sampling fragment position in view space
    vec3 vv = getViewSpacePosition(iuv);

    // Take a xy-random base for more variative TBN basis
    vec3 baseRv = vec3(
        goldNoise(iuv * rc++) * 2.0 - 1.0, // x = -1.0 to 1.0
        goldNoise(iuv * rc++) * 2.0 - 1.0, // y = -1.0 to 1.0
        0.0
    );

    // Gramm-Schmidt process to orthogonalize tangent respect to baseRv
    vec3 tv = normalize(baseRv - nv * dot(baseRv, nv));
    vec3 bv = cross(nv, tv);
    // Calculate TBN rotation matrix from tangent space to view space
    mat3 TBN = mat3(tv, bv, nv); 

    for (int i = 0; i < LITE3D_SSAO_MAX_DEPTH_SAMPLES; ++i)
    {
        // Random test sample in half hemisphere at tangent space 
        vec3 probeRay = normalize(vec3(
            goldNoise(iuv * rc++) * 2.0 - 1.0, // x = -1.0 to 1.0
            goldNoise(iuv * rc++) * 2.0 - 1.0, // y = -1.0 to 1.0
            goldNoise(iuv * rc++)              // z = 0.0 to 1.0
        ));

        // set sampler closer and closer to actual fragment 
        float sampleScale = float(i) / float(LITE3D_SSAO_MAX_DEPTH_SAMPLES);
        probeRay *= lerp(0.1f, 1.0f, sampleScale * sampleScale);
        // transform sample to view space using TBN and calc sample position in world space
        probeRay = vv + (TBN * probeRay) * AORadius;

        // Get Screen UV coordinate to sample surface depth in GBuffer
        // Convert sample view pos to clip-space
        vec2 offsetUV = viewPositionToUV(probeRay);
        // Get surface world position at sample from GBuffer and translate to view space
        float surfaceDepth = getViewSpacePosition(offsetUV).z;
        // Check sample depth respect to surface depth in view-space with range check
        //float rangeCheck = abs(vv.z - surfaceDepth) < AORadius ? 1.0 : 0.0;
        float rangeCheck = smoothstep(0.0, 1.0, AORadius / max(abs(vv.z - surfaceDepth), FLT_EPSILON));
        aoFactor += (surfaceDepth >= (probeRay.z + LITE3D_SSAO_SAMPLE_BIAS) ? 1.0 : 0.0) * rangeCheck;
    }

    aoFactor = 1.0 - pow(aoFactor / float(LITE3D_SSAO_MAX_DEPTH_SAMPLES), LITE3D_SSAO_POWER);
    outColor = vec4(vec3(aoFactor), 1.0);
}
