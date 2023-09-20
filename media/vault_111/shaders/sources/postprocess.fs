#include "samples:shaders/sources/common/version.def"

uniform sampler2D combined;
uniform float GammaFactor;

void main()
{
    vec4 finalColor = texture2D(combined, iuv);

    // apply gamma correction 
    finalColor.rgb = pow(finalColor.rgb, vec3(1/GammaFactor));
    gl_FragColor = vec4(finalColor.rgb, 1.0);
}