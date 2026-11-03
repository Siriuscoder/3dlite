#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

#include "samples:shaders/sources/pipeline/forward_pass/lighting.fs"

void main()
{
    Surface surface = restoreSurface(iuv);
    AngularInfo angular = AngularInfo(vec3(0), vec3(0), vec3(0), 0.0, false, 0.0, 0.0, 0.0, 0.0, 0.0);
    angularInfoInit(angular, surface);
    // Compute total illumination 
    fragColor = vec4(ComputeLighting(surface, angular), 1.0);
}
