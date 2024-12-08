#include "samples:shaders/sources/common/common_inc.glsl"

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal

vec3 ComputeIllumination(in Surface surface);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, vec3(0.0), vec3(0.0));
    // Compute total illumination 
    fragColor = vec4(ComputeIllumination(surface), surface.material.alpha);
}
