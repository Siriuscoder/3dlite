#version 330

in vec2 ivertex;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 uv;

void main()
{
    uv = ivertex;
    gl_Position = projectionMatrix * viewMatrix * vec4(ivertex.xy, 0.0, 1.0);
}