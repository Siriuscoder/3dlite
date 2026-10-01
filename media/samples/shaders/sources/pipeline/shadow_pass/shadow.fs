#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

void main()
{
    surfaceAlphaClip(iuv);

    float depth = gl_FragCoord.z;
    fragColor = vec4(depth, depth * depth, 0.0, 0.0);
}
