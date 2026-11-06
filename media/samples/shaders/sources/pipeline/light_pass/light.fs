#define LITE3D_DISABLE_INVOCATION_METHOD
#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

#include "samples:shaders/sources/pipeline/forward_pass/lighting.fs"

void main()
{
    Surface surface;
    if (!restoreSurface(iuv, surface))
    {
        discard;
    }

    AngularInfo angular;
    angularInfoInit(angular, surface);
    // Compute total illumination 
    fragColor = vec4(ComputeLighting(surface, angular), 1.0);
}
