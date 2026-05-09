#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2DArray GBuffer;
uniform float AORadius;
uniform int FrameNumber;

in vec2 iuv;
out vec4 outColor;

#define LITE3D_SSAO_MAX_SAMPLES 64

const vec3 SSAO_SAMPLES[LITE3D_SSAO_MAX_SAMPLES] = vec3[](

    vec3( 0.0482, -0.1231, 0.9912),
    vec3(-0.1865,  0.0914, 0.9782),
    vec3( 0.2124,  0.1748, 0.9615),
    vec3(-0.0713, -0.2875, 0.9551),
    vec3( 0.3312, -0.0528, 0.9422),
    vec3(-0.2944,  0.2483, 0.9230),
    vec3( 0.1092,  0.3824, 0.9174),
    vec3(-0.4018, -0.1117, 0.9087),

    vec3( 0.4286,  0.2035, 0.8801),
    vec3(-0.1724, -0.4521, 0.8750),
    vec3( 0.0168,  0.5213, 0.8532),
    vec3(-0.5155,  0.0584, 0.8449),
    vec3( 0.3620, -0.4014, 0.8411),
    vec3(-0.2928,  0.4761, 0.8286),
    vec3( 0.5612,  0.1075, 0.8207),
    vec3(-0.4760, -0.3184, 0.8199),

    vec3( 0.1854,  0.6032, 0.7758),
    vec3(-0.6104,  0.2147, 0.7625),
    vec3( 0.5128, -0.4027, 0.7583),
    vec3(-0.0864, -0.6483, 0.7565),
    vec3( 0.6641,  0.2912, 0.6892),
    vec3(-0.3845,  0.6201, 0.6840),
    vec3( 0.2714, -0.6792, 0.6818),
    vec3(-0.7032, -0.1221, 0.7004),

    vec3( 0.7031, -0.2420, 0.6687),
    vec3(-0.2374,  0.7342, 0.6361),
    vec3( 0.0084, -0.7821, 0.6231),
    vec3(-0.7740,  0.1962, 0.6018),
    vec3( 0.5918,  0.5451, 0.5938),
    vec3(-0.5297, -0.6172, 0.5819),
    vec3( 0.8075,  0.0914, 0.5826),
    vec3(-0.1218,  0.8215, 0.5570),

    vec3( 0.4018, -0.7352, 0.5461),
    vec3(-0.8421, -0.0821, 0.5328),
    vec3( 0.7224,  0.4638, 0.5126),
    vec3(-0.4531,  0.7345, 0.5050),
    vec3( 0.1692, -0.8501, 0.4984),
    vec3(-0.8825,  0.2841, 0.3748),
    vec3( 0.8428, -0.3510, 0.4084),
    vec3(-0.2614, -0.8805, 0.3943),

    vec3( 0.5438,  0.7621, 0.3518),
    vec3(-0.7521,  0.5425, 0.3741),
    vec3( 0.9218,  0.1182, 0.3692),
    vec3(-0.0891,  0.9281, 0.3614),
    vec3( 0.3478, -0.8722, 0.3434),
    vec3(-0.9411, -0.1714, 0.2912),
    vec3( 0.7815, -0.5478, 0.2991),
    vec3(-0.5142, -0.8121, 0.2761),

    vec3( 0.6774,  0.6892, 0.2574),
    vec3(-0.8561,  0.4411, 0.2695),
    vec3( 0.9674, -0.0421, 0.2497),
    vec3(-0.0125,  0.9714, 0.2371),
    vec3( 0.4921, -0.8464, 0.2034),
    vec3(-0.9725, -0.1121, 0.2040),
    vec3( 0.8624, -0.4762, 0.1715),
    vec3(-0.6475, -0.7428, 0.1704),

    vec3( 0.7921,  0.5932, 0.1422),
    vec3(-0.9135,  0.3782, 0.1485),
    vec3( 0.9914,  0.0124, 0.1302),
    vec3(-0.1025,  0.9861, 0.1308),
    vec3( 0.5912, -0.7974, 0.1214),
    vec3(-0.9854, -0.0915, 0.1431),
    vec3( 0.9021, -0.4138, 0.1182),
    vec3(-0.7528, -0.6465, 0.1231)
);

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
    // sampling fragment normal in view space
    vec3 nv = getViewSpaceNormal(iuv);
    // sampling fragment position in view space
    vec3 vv = getViewSpacePosition(iuv);
    // Adaptive sample bias
    float NdotV = abs(dot(nv, normalize(-vv)));
    float bias = mix(LITE3D_SSAO_ADAPTIVE_BIAS_MIN, LITE3D_SSAO_ADAPTIVE_BIAS_MAX, 1.0 - NdotV);

    vec2 hran = Halton2D(int(iuv.x * FrameNumber) + int(iuv.y * FrameNumber));
    // Take a xy-random base for more variative TBN basis
    vec3 baseRv = vec3(
        hran * 2.0 - 1.0, // x,y = -1.0 to 1.0
        0.0
    );

    // Gramm-Schmidt process to orthogonalize tangent respect to baseRv
    vec3 tv = normalize(baseRv - nv * dot(baseRv, nv));
    vec3 bv = cross(nv, tv);
    // Calculate TBN rotation matrix from tangent space to view space
    mat3 TBN = mat3(tv, bv, nv); 

    for (int i = 0; i < LITE3D_SSAO_MAX_SAMPLES; ++i)
    {
        // Random test sample in half hemisphere at tangent space 
        float t = float(i) / LITE3D_SSAO_MAX_SAMPLES;
        float scale = mix(0.1, 1.0, t * t);
        vec3 probeRay = SSAO_SAMPLES[i] * scale;
        // transform sample to view space using TBN and calc sample position in world space
        probeRay = vv + (TBN * probeRay) * AORadius;
        // Get Screen UV coordinate to sample surface depth in GBuffer
        // Convert sample view pos to clip-space
        vec2 offsetUV = viewPositionToUV(probeRay);
        // Get surface world position at sample from GBuffer and translate to view space
        float probeDepth = getViewSpacePosition(offsetUV).z;
        // Check sample depth respect to surface depth in view-space with range check
        float rangeCheck = smoothstep(0.0, 1.0, AORadius / max(abs(vv.z - probeDepth), FLT_EPSILON));
        aoFactor += (probeDepth >= (probeRay.z + bias) ? 1.0 : 0.0) * rangeCheck;
    }

    aoFactor = pow(1.0 - (aoFactor / float(LITE3D_SSAO_MAX_SAMPLES)), LITE3D_SSAO_POWER);
    outColor = vec4(aoFactor, 0.0, 0.0, 1.0);
}
