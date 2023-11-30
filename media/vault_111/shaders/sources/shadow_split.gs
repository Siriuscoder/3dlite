#include "samples:shaders/sources/common/version.def"

#define MAX_SHADOW_LAYERS 4

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[MAX_SHADOW_LAYERS];
};

layout(std140) uniform ShadowIndex
{
    ivec4 shadowIndex[MAX_SHADOW_LAYERS / 4 + 1];
};

void main()
{
    int count = shadowIndex[0].x;
    for (int i = 1; i <= count; i++)
    {
        int index = shadowIndex[i/4][int(mod(i, 4))];
        gl_Layer = index;

        for (int j = 0; j < 3; ++j)
        {
            gl_Position = shadowMat[index] * gl_in[j].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}