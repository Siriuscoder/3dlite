#include "samples:shaders/sources/common/common_inc.glsl"

#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY__PRESET 39

#include "samples:shaders/sources/antialiasing/FXAA-3.11.glsl"

uniform sampler2D InputImage;

out vec4 fragColor;
in vec2 iuv;

const float subPix           = 0.9;    // the amount of sub-pixel aliasing removal. This can effect sharpness.
const float edgeThreshold    = 0.125;  // the minimum amount of local contrast required to apply algorithm.
const float edgeThresholdMin = 0.0625; // trims the algorithm from processing darks

void main()
{
    vec2 srcTexelSize = 1.0 / textureSize(InputImage, 0);
    vec4 zero = vec4(0.0);

    vec3 color = FxaaPixelShader(
        iuv, zero, InputImage, InputImage, InputImage,
        srcTexelSize, zero, zero, zero,
        subPix,
        edgeThreshold,
        edgeThresholdMin,
        0, 0, 0, zero
    ).rgb;

    // Dithering
    color = ditherBayer(gl_FragCoord.xy, color);

    // Final Color
    fragColor = vec4(color, 1.0);
}
