#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D Combined;
uniform sampler2D Bloom;
uniform vec3 ScreenResolution;

out vec4 fragColor;

in vec2 iuv;
in vec2 irgbNW;
in vec2 irgbNE;
in vec2 irgbSW;
in vec2 irgbSE;
in vec2 irgbM;

const float BloomStrength = 0.04;

vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
    vec2 v_rgbNW, vec2 v_rgbNE, 
    vec2 v_rgbSW, vec2 v_rgbSE, 
    vec2 v_rgbM); 

void main()
{
    // FSAA 
    vec3 hdr = fxaa(Combined, iuv * ScreenResolution.xy, ScreenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM).rgb;
    // BLOOM
    hdr = mix(hdr, texture(Bloom, iuv).rgb, BloomStrength);
    // Exposure tone mapping
    vec4 ldr = vec4(exposureTonemapping(hdr), 1.0);
    // Color correction
    ldr = contrastMatrix() * saturationMatrix() * ldr;
    // Gamma correction 
    ldr.rgb = linearToSRGB(ldr.rgb);
    // Dithering
    ldr.rgb = ditherBayer(ldr.rgb);
    // Final Color
    fragColor = vec4(ldr.rgb, 1.0);
}