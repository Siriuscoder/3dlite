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

const float bloomStrength = 0.04f;

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

void main()
{
    vec3 hdr = fxaa(combined, iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM).xyz;
    vec3 blm = texture(bloom, iuv).xyz;
    hdr = mix(hdr, blm, vec3(bloomStrength));
    // Exposure tone mapping
    vec3 ldr = ExposureTonemapping(hdr, 1.1);
    // gamma correction 
    ldr = pow(ldr, vec3(1.0 / GammaFactor));

    fragColor = vec4(ldr, 1.0);
}