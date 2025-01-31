#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

void main()
{
    surfaceAlphaClip(iuv);
}
