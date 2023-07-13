#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuse;
uniform float animcounter; // [0..1]

in vec2 iuv;
in vec3 ivv;
in vec3 wnorm;

out vec4 fragColor;

const vec3 fogColor = vec3(0.0, 0.95, 0.89);

vec3 fogFunc(vec3 frag, vec3 fc)
{
    const float density = 0.0005;
    const float LOG2 = 1.442695;

    float zfactor = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = clamp(exp2(-density * density * zfactor * zfactor * LOG2), 0.0, 1.0);
    return mix(fc, frag, fogFactor);
}

void main()
{
    /* calculate fog factor */
    fragColor = vec4(fogFunc(texture2D(diffuse, vec2(iuv.x, iuv.y + animcounter)).rgb, fogColor), 1.0);
}