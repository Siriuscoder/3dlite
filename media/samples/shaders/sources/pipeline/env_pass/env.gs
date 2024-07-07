#include "samples:shaders/sources/common/version.def"

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140) uniform CubeTransform
{
    mat4 projViewCubeMatrices[6];
};

in vec2 iuv_g[];
in mat3 itbn_g[];

out vec2 iuv;    // UVs
out vec4 ivv;    // world-space position
out mat3 itbn;   // The matrix that transforms vector from tangent space to world space 

void main()
{
    for (int i = 0; i < 6; ++i)
    {
        gl_Layer = i;

        for (int j = 0; j < 3; ++j)
        {
            ivv = gl_in[j].gl_Position;
            iuv = iuv_g[j];
            itbn = itbn_g[j];
            gl_Position = projViewCubeMatrices[i] * ivv;
            EmitVertex();
        }
        EndPrimitive();
    }
}