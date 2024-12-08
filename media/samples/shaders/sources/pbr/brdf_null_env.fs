#include "samples:shaders/sources/common/common_inc.glsl"

#ifndef LITE3D_BASE_AMBIENT_LIGHT
#define LITE3D_BASE_AMBIENT_LIGHT vec3(0.0)
#endif

vec3 ComputeIndirect(in Surface surface, in AngularInfo angular)
{
    return LITE3D_BASE_AMBIENT_LIGHT;
}
