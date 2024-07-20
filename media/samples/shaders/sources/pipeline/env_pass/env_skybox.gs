#include "samples:shaders/sources/common/version.def"

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140) uniform CubeTransform
{
    mat4 projViewCubeMatrices[6];
};

in vec3 iuv_g[];

out vec3 iuv;    // UVs

void main()
{
    for (int i = 0; i < 6; ++i)
    {
        gl_Layer = i;

        for (int j = 0; j < 3; ++j)
        {
            // clip-space position
            vec4 vs = projViewCubeMatrices[i] * gl_in[j].gl_Position;
            gl_Position = vs.xyww;
            // UVs
            iuv = iuv_g[j];
            EmitVertex();
        }
        EndPrimitive();
    }
}