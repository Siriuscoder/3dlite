#include "samples:shaders/sources/common/version.def"

layout(triangles) in;
layout(triangle_strip, max_vertices=9) out;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[3];
};

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Layer = i;
        for (int j = 0; j < 3; ++j)
        {
            gl_Position = shadowMat[i] * gl_in[j].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}