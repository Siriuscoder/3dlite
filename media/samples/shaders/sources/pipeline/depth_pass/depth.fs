#include "samples:shaders/sources/common/material_inc.glsl"

in vec2 iuv;

void main()
{
    Surface surface = makeSurface(iuv, vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));
    surfaceAlphaClip(surface);
}
