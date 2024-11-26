#ifdef BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

in vec2 iuv;

void main()
{
    Surface surface = makeSurface(iuv, vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));
    surfaceAlphaClip(surface);
}
