#version 330

layout(location = 0) in vec4 vertex;
layout(location = 1) in mat4 modelMatrix; // per instance matrix

out mat4 wm;

void main()
{
    wm = modelMatrix;
    gl_Position = vertex;
}
