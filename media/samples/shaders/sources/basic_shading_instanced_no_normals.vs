layout(location = 0) in vec4 v;
layout(location = 1) in mat4 model; // per instance matrix

out mat4 wm;

void main()
{
    wm = model;
    gl_Position = v;
}
