#version 330

layout(location = 0) in vec3 vertex;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 iuv;

void main()
{
    // texture coordinate 
    iuv = vertex.xzy * -1.0;
    vec4 pos = projectionMatrix * viewMatrix * vec4(vertex, 0.0);
    gl_Position = pos.xyww;
}