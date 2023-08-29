#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 v;
layout(location = 1) in vec3 n;
layout(location = 2) in vec2 tc;

out vec2 uv;
out vec3 wn;
out vec3 wv;

// common functions
vec4 rtransform(vec4 v1);
vec4 wtransform(vec4 v1);
vec3 rntransform(vec3 normal);

void main()
{
    uv = tc;
    wn = rntransform(n);
    vec4 wvp = wtransform(v);
    wv = wvp.xyz / wvp.w;
    gl_Position = rtransform(v);
}