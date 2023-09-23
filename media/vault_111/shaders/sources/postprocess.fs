#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform float GammaFactor;

in vec2 iuv;

void main()
{
    vec3 hdr = texture(combined, iuv).xyz;
    // tonemapping
    vec3 ldr = hdr / (hdr + vec3(1.0));
    // gamma correction 
    ldr = pow(ldr, vec3(1.0 / GammaFactor));

    gl_FragColor = vec4(ldr, 1.0);
}