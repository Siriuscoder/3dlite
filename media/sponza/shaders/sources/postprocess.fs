#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform int SwitchView;
uniform float GammaFactor;
uniform sampler2D shadowmap;

in vec2 irgbNW;
in vec2 irgbNE;
in vec2 irgbSW;
in vec2 irgbSE;
in vec2 irgbM;

out vec4 fragColor;

vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
    vec2 v_rgbNW, vec2 v_rgbNE, 
    vec2 v_rgbSW, vec2 v_rgbSE, 
    vec2 v_rgbM);

const vec3 correction = vec3(0.95, 0.99, 1.04);

vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) 
{
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.3333333333333333) * direction;
    color += texture(image, uv) * 0.29411764705882354;
    color += texture(image, uv + (off1 / resolution)) * 0.35294117647058826;
    color += texture(image, uv - (off1 / resolution)) * 0.35294117647058826;
    return color; 
}

void main()
{
    vec2 sr = textureSize(combined, 0);
    vec4 finalColor = vec4(0.0);
    
    if (SwitchView == 0)
    {
        finalColor = fxaa(combined, gl_FragCoord.xy, sr, irgbNW, irgbNE, irgbSW, irgbSE, irgbM);
        // apply gamma correction 
        finalColor.rgb = pow(finalColor.rgb, vec3(1/GammaFactor)) * correction;
    }
    else
    {
        finalColor = texture(shadowmap, gl_FragCoord.xy / sr).rrrr;
    }
        
    //if (length(finalColor) >= 1.2)
    //    finalColor = blur5(combined, iuv, sr, vec2(1, 1));
    fragColor = vec4(finalColor.rgb, 1.0);
}