#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;

void main()
{
    surfaceAlphaClip(iuv);
}
