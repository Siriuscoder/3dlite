#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

out vec2 iuv;
out vec3 ivn;

void main()
{
    // texture coordinate 
    iuv = uv;
    ivn = normalize(normalMatrix * normal);
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vec4(vertex, 1);
    gl_Position = projectionMatrix * viewMatrix * wv;
}