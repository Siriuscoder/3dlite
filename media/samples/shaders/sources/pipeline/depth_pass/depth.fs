#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
#include "samples:shaders/sources/bindless/material_inc.glsl"
#else
#include "samples:shaders/sources/common/material_inc.glsl"
#endif

in vec2 iuv;

void main()
{
    surfaceAlphaClip(iuv);
}
