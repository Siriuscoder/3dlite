#include "samples:shaders/sources/common/version.def"

uniform sampler2D diffuseSampler;
uniform vec4 fogColor;

in vec2 uv;
in vec3 wn;

out vec4 fragcolor;

vec4 fColor;
float density = 0.0005;
const float LOG2 = 1.442695;

void main()
{
    vec3 lightDir = vec3(-1.0, -1.0, 1.0);
    float nDotL = clamp(dot(wn, lightDir), 0.2, 1);
    vec4 diffColor = texture2D(diffuseSampler, uv.st) * nDotL;
    
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = exp2(-density * density * z * z * LOG2);
    fogFactor = clamp(fogFactor, 0.0, 1.0);    
    fragcolor = mix(fogColor, diffColor, fogFactor);
}