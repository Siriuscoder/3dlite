uniform sampler2D combined;
uniform int FXAA;
uniform float GammaFactor;
uniform vec3 screenResolution;

in vec2 iuv;
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

void main()
{
    // apply FXAA
    vec4 finalColor = FXAA > 0 ? fxaa(combined, iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM) : texture(combined, iuv);

    // apply gamma correction 
    finalColor.rgb = pow(finalColor.rgb, vec3(1/GammaFactor));
    fragColor = vec4(finalColor.rgb * correction, 1.0);
}