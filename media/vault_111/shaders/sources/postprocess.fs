#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform sampler2D bloom;
uniform float GammaFactor;
uniform vec3 screenResolution;

out vec4 fragColor;

in vec2 iuv;
in vec2 irgbNW;
in vec2 irgbNE;
in vec2 irgbSW;
in vec2 irgbSE;
in vec2 irgbM;

const float BloomStrength = 0.04;
const float Contrast = 1.002;
const float Saturation = 1.18;

vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
    vec2 v_rgbNW, vec2 v_rgbNE, 
    vec2 v_rgbSW, vec2 v_rgbSE, 
    vec2 v_rgbM); 

// Reinhard tone mapping
vec3 ReinhardTonemapping(vec3 x)
{
    return x / (x + 1.0);
}

// exposure tone mapping
vec3 ExposureTonemapping(vec3 x, float exposure)
{
    return 1.0 - exp(-x * exposure);
}

mat4 contrastMatrix(float contrast)
{
	float t = (1.0 - contrast) / 2.0;
    return mat4(contrast, 0, 0, 0,
                0, contrast, 0, 0,
                0, 0, contrast, 0,
                t, t, t, 1);
}

mat4 saturationMatrix(float saturation)
{
    vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
    float oneMinusSat = 1.0 - saturation;

    vec3 red = vec3(luminance.x * oneMinusSat);
    red += vec3(saturation, 0, 0);
    
    vec3 green = vec3(luminance.y * oneMinusSat);
    green += vec3(0, saturation, 0);
    
    vec3 blue = vec3(luminance.z * oneMinusSat);
    blue += vec3(0, 0, saturation);
    
    return mat4(red,     0,
                green,   0,
                blue,    0,
                0, 0, 0, 1);
}

void main()
{
    // FSAA 
    vec3 hdr = fxaa(combined, iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM).rgb;
    // Apply bloom
    vec3 blm = texture(bloom, iuv).rgb;
    hdr = mix(hdr, blm, vec3(BloomStrength));
    // Exposure tone mapping
    vec4 ldr = vec4(ExposureTonemapping(hdr, 2.4), 1.0);
    // Color correction
    ldr = contrastMatrix(Contrast) * saturationMatrix(Saturation) * ldr;
    // Gamma correction 
    ldr.rgb = pow(ldr.rgb, vec3(1.0 / GammaFactor));
    // Final Color
    fragColor = vec4(ldr.rgb, 1.0);
}