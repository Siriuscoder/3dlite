#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in mat4 modelMatrix;

uniform mat4 projViewMatrix;

void main()
{
    gl_Position = projViewMatrix * modelMatrix * vec4(vertex, 1.0);
}