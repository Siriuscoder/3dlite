#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;

uniform mat4 modelMatrix;

out vec2 iuv_g;

void main()
{
    // texture coordinate 
    iuv_g = vertex * -1.0;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vec4(vertex, 0.0);
}
