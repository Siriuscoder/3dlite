#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

vec3 ComputeIllumination(in Surface surface);

void main()
{
    Surface surface = restoreSurface(iuv);
    // Compute total illumination 
    fragColor = vec4(ComputeIllumination(surface), 1.0);
}
