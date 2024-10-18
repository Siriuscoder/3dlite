layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 projViewMatrix;
uniform mat4 modelMatrix;

out vec2 iuv;

void main()
{
    iuv = uv;
    vec4 wv = modelMatrix * vertex;
    gl_Position = projViewMatrix * wv;
}