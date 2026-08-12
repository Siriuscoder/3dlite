#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

vec3 ComputeIllumination(in Surface surface, in AngularInfo angular);

void main()
{
    Surface surface = restoreSurface(iuv);
    AngularInfo angular = AngularInfo(vec3(0), vec3(0), 0.0, false, 0.0, 0.0, 0.0, 0.0, 0.0);
    angularInfoInit(angular, surface);
    // Compute total illumination 
    fragColor = vec4(ComputeIllumination(surface, angular), 1.0);
}
