#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;

uniform mat4 screenMatrix;

void main()
{
    gl_Position = screenMatrix * vertex;
}