#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform sampler2D bloom;
uniform float gamma;
uniform float exposure;
uniform float contrast;
uniform float saturation;
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
vec3 ExposureTonemapping(vec3 x, float e)
{
    return 1.0 - exp(-x * e);
}

mat4 contrastMatrix(float c)
{
	float t = (1.0 - c) / 2.0;
    return mat4(c, 0, 0, 0,
                0, c, 0, 0,
                0, 0, c, 0,
                t, t, t, 1);
}

mat4 saturationMatrix(float s)
{
    vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
    float oneMinusSat = 1.0 - s;

    vec3 red = vec3(luminance.x * oneMinusSat);
    red += vec3(s, 0, 0);
    
    vec3 green = vec3(luminance.y * oneMinusSat);
    green += vec3(0, s, 0);
    
    vec3 blue = vec3(luminance.z * oneMinusSat);
    blue += vec3(0, 0, s);
    
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
    vec4 ldr = vec4(ExposureTonemapping(hdr, exposure), 1.5);
    // Color correction
    ldr = contrastMatrix(contrast) * saturationMatrix(saturation) * ldr;
    // Gamma correction 
    ldr.rgb = pow(ldr.rgb, vec3(1.0 / gamma));
    // Final Color
    fragColor = vec4(ldr.rgb, 1.0);
}