layout(location = 0) in vec4 v;
layout(location = 1) in vec2 tc;

out vec2 uv;

// common functions
vec4 rtransform(vec4 v1);

void main()
{
    uv = tc;
    gl_Position = rtransform(v);
}