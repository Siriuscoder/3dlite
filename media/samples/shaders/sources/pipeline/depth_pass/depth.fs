#include "samples:shaders/sources/common/material_inc.glsl"

in vec2 iuv;

void main()
{
    Surface surface = makeSurface(iuv);
    surfaceAlphaClip(surface);
}