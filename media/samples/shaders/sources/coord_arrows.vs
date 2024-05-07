#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in mat4 modelMatrix;

uniform mat4 projViewMatrix;

out vec3 icolor;

void main()
{
    icolor = floor(clamp(vertex.xyz, 0.0, 1.0));
    gl_Position = projViewMatrix * modelMatrix * vertex;
}