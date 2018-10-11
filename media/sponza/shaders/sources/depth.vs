#include "sponzamat:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vec4(vertex, 1);
    gl_Position = projectionMatrix * viewMatrix * wv;
}