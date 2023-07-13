#include "samples:shaders/sources/common/version.def"

uniform sampler2D lightMap;
uniform sampler2D diffuseMap;

in vec2 iuv;
out vec4 fragColor;

const vec3 ambient = vec3(0.07, 0.07, 0.07);

void main()
{
    /* fragment coordinate */
    vec4 diff = texture2D(diffuseMap, iuv);
    /* check fragment not shaded or self-illum material */
    if (diff.w == 1.0)
    {
        fragColor = vec4(diff.xyz, 1.0);
        return;
    }

    vec3 linear = texture2D(lightMap, iuv).rgb;
    /* result color in LDR */
    fragColor = vec4(diff.rgb * (ambient + linear), 1.0);
}