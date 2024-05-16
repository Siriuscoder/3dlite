#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 vertex;

uniform mat4 modelMatrix;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[1];
};

void main()
{
    gl_Position = shadowMat[0] * modelMatrix * vertex;
}