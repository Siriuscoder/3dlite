#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform float GammaFactor;
uniform vec3 screenResolution;

in vec2 iuv;
in vec2 irgbNW;
in vec2 irgbNE;
in vec2 irgbSW;
in vec2 irgbSE;
in vec2 irgbM;

vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
    vec2 v_rgbNW, vec2 v_rgbNE, 
    vec2 v_rgbSW, vec2 v_rgbSE, 
    vec2 v_rgbM); 

vec3 tonemapCoeff(vec3 x)
{
    float _A = 0.15;
    float _B = 0.50;
    float _C = 0.10;
    float _D = 0.20;
    float _E = 0.02;
    float _F = 0.30;

    return ((x*(_A*x+_C*_B)+_D*_E)/(x*(_A*x+_B)+_D*_F))-_E/_F;
}

vec3 tonemapping(vec3 x)
{
    vec3 coeffone = tonemapCoeff(vec3(1.0));

    x = tonemapCoeff(x);
    return x / (x + coeffone);
}

void main()
{
    vec3 hdr = fxaa(combined, iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM).xyz;
    // tonemapping
    vec3 ldr = tonemapping(hdr);
    // gamma correction 
    ldr = pow(ldr, vec3(1.0 / GammaFactor));

    gl_FragColor = vec4(ldr, 1.0);
}