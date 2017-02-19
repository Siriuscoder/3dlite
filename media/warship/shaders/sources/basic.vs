#version 140

in vec3 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec3 vnormal;
out vec2 uv;

void main()
{
    uv = texCoordAttr;
    vnormal = normalMatrix * normalAttr;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexAttr, 1);
}