#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;

uniform mat4 screenMatrix;

out vec3 iuv;

void main()
{
    // texture coordinate 
    iuv = vertex.xzy * -1.0;
    vec4 pos = screenMatrix * vec4(vertex, 0.0);
    gl_Position = pos.xyww;
}