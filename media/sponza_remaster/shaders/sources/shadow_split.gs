#include "sponza:shaders/sources/inc/common.def"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[MAX_SHADOW_LAYERS];
};

layout(std140) uniform ShadowIndex
{
    ivec4 shadowIndex[MAX_SHADOW_LAYERS / 4 + 1];
};

in vec2 vsUV[];
out vec2 iuv;

void main()
{
    int count = shadowIndex[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = shadowIndex[i/4][int(mod(i, 4))];
        gl_Layer = index;

        for (int j = 0; j < 3; ++j)
        {
            gl_Position = shadowMat[index] * gl_in[j].gl_Position;
            iuv = vsUV[j];
            EmitVertex();
        }
        EndPrimitive();
    }
}