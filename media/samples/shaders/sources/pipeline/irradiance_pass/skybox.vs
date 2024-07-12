#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;

void main()
{
    gl_Position = vec4(vertex, 0.0);
}
