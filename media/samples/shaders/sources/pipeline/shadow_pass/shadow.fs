#include "samples:shaders/sources/common/common_inc.glsl"

in vec2 iuv;
out vec4 fragColor;

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

#include "samples:shaders/sources/shadow/vsm.glsl"

void main()
{
    surfaceAlphaClip(iuv);

    float warpedDepth = WarpDepth(gl_FragCoord.z, GetEVSMExponent());
    fragColor = vec4(warpedDepth, warpedDepth * warpedDepth, 0.0, 0.0);
}
