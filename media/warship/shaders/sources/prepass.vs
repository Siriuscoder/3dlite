#version 330

in vec3 ivertex;
in vec3 inormal;
in vec2 iuv;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec4 vv;
out vec3 vn;
out vec2 uv;

void main()
{
    uv = iuv;
    vn = normalMatrix * inormal;
    vv = modelMatrix * vec4(ivertex, 1);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(ivertex, 1);
}