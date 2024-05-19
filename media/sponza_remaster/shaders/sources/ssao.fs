#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

#define SSAO_MAX_DEPTH_SAMPLES 80
#define SSAO_SAMPLE_BIAS 0.085

uniform sampler2DArray GBuffer;
uniform mat4 CameraView;
uniform mat4 CameraProjection;
uniform float AORadius;
uniform float RandomSeed; /* 0.0 - 1.0 */

in vec2 iuv;
out vec4 outColor;

vec3 viewSpacePosition(vec2 uv)
{
    vec4 pos = CameraView * vec4(texture(GBuffer, vec3(uv, 0)).xyz, 1.0);
    return pos.xyz / pos.w;
}

vec3 viewSpaceNormal(vec2 uv)
{
    vec3 nw = texture(GBuffer, vec3(uv, 1)).xyz;
    // Non shaded fragment
    if (fiszero(nw))
        discard;

    return normalize((CameraView * vec4(nw, 0.0)).xyz);
}

void main()
{
    float aoFactor = 0.0;
    float rc = 1.0; // For more randomization
    // sampling fragment normal in view space
    vec3 nv = viewSpaceNormal(iuv);
    // sampling fragment position in view space
    vec3 vv = viewSpacePosition(iuv);

    // Take a xy-random base for more variative TBN basis
    vec3 baseRv = vec3(
        goldNoise(iuv * rc++, RandomSeed) * 2.0 - 1.0, // x = -1.0 to 1.0
        goldNoise(iuv * rc++, RandomSeed) * 2.0 - 1.0, // y = -1.0 to 1.0
        0.0
    );

    // Gramm-Schmidt process to orthogonalize tangent respect to baseRv
    vec3 tv = normalize(baseRv - nv * dot(baseRv, nv));
    vec3 bv = cross(nv, tv);
    // Calculate TBN rotation matrix from tangent space to view space
    mat3 TBN = mat3(tv, bv, nv); 

    for (int i = 0; i < SSAO_MAX_DEPTH_SAMPLES; ++i)
    {
        // Random test sample in half hemisphere at tangent space 
        vec3 sample = normalize(vec3(
            goldNoise(iuv * rc++, RandomSeed) * 2.0 - 1.0, // x = -1.0 to 1.0
            goldNoise(iuv * rc++, RandomSeed) * 2.0 - 1.0, // y = -1.0 to 1.0
            goldNoise(iuv * rc++, RandomSeed) // z = 0.0 to 1.0
        ));

        // set sampler closer and closer to actual fragment 
        float sampleScale = float(i) / float(SSAO_MAX_DEPTH_SAMPLES);
        sample *= lerp(0.1f, 1.0f, sampleScale * sampleScale);
        // transform sample to view space using TBN and calc sample position in world space
        sample = vv + (TBN * sample) * AORadius;

        // Get Screen UV coordinate to sample surface depth in GBuffer
        // Convert sample view pos to clip-space
        vec4 offsetUV = CameraProjection * vec4(sample, 1.0);
        offsetUV /= offsetUV.w;             // perspective divide
        offsetUV = offsetUV * 0.5 + 0.5;    // transform to range 0.0 - 1.0  
        // Get surface world position at sample from GBuffer and translate to view space
        float surfaceDepth = viewSpacePosition(offsetUV.xy).z;
        // Check sample depth respect to surface depth in view-space with range check
        //float rangeCheck = abs(vv.z - surfaceDepth) < AORadius ? 1.0 : 0.0;
        float rangeCheck = smoothstep(0.0, 1.0, AORadius / max(abs(vv.z - surfaceDepth), FLT_EPSILON));
        aoFactor += (surfaceDepth >= (sample.z + SSAO_SAMPLE_BIAS) ? 1.0 : 0.0) * rangeCheck;
    }

    aoFactor = 1.0 - pow(aoFactor / float(SSAO_MAX_DEPTH_SAMPLES), 0.4);
    outColor = vec4(vec3(aoFactor), 1.0);
}
