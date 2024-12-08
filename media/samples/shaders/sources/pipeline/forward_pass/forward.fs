#include "samples:shaders/sources/common/common_inc.glsl"

out vec4 fragColor;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

vec3 ComputeIllumination(in Surface surface);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);
    // Compute total illumination 
    fragColor = vec4(ComputeIllumination(surface), surface.material.alpha);
}
