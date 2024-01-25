#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

#define MAX_DEPTH_SAMPLES 64
const float PHI = 1.61803398874989484820459; // Φ = Golden Ratio 

uniform sampler2DArray GBuffer;
uniform mat4 MainCameraViewProjection;
uniform float SSAORadius;
uniform float RandomSeed; /* 0.0 - 1.0 */

in vec2 iuv;
out vec4 fragColor;

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy, float seed)
{
    return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void main()
{
    // sampling normal in world space from fullscreen normal map
    vec4 nw = texture(GBuffer, vec3(iuv, 1));
    // Non shaded fragment
    if (fiszero(nw.xyz))
        discard;

    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(GBuffer, vec3(iuv, 0)).xyz;

    // Random vector with zero Z
    vec3 baseRotationVec = normalize(vec3(1.0, 1.0, 0.0));
    // Calculate rotation matrix from tangent space to world space
    // Gramm-Schmidt process to orthogonalize tangent respect to baseRotationVec
    vec3 tw = normalize(baseRotationVec - nw * dot(baseRotationVec, nw));
    vec3 bw = cross(normal, tangent);
    mat3 TBN = mat3(tw, bw, nw); 
    int rc = 0;

    for (int i = 0; i < MAX_DEPTH_SAMPLES; ++i)
    {
        // Random test sample in half hemisphere at tangent space 
        // Z axis is perpendicular to surface
        vec3 sample = normalize(vec3(
            goldNoise(iuv, fract(RandomSeed * float(rc++))) * 2.0 - 1.0, // x = -1.0 to 1.0
            goldNoise(iuv, fract(RandomSeed * float(rc++))) * 2.0 - 1.0, // y = -1.0 to 1.0
            goldNoise(iuv, fract(RandomSeed * float(rc++))) // z = 0.0 to 1.0
        ));

        // set sampler closer and closer to actual fragment 
        float sampleScale = float(i) / float(MAX_DEPTH_SAMPLES);
        sample *= lerp(0.1f, 1.0f, sampleScale * sampleScale);
        // transform sample to world space using TBN and calc sample position in world space
        sample = vw + (TBN * sample) * SSAORadius;

    }
}
