#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;

uniform mat4 screenMatrix;

void main()
{
    // vertex coordinate in world space 
    gl_Position = screenMatrix * vec4(vertex, 1);
}