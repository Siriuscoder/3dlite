#include "samples:shaders/sources/common/version.def"

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std140) uniform CubeTransform
{
    mat4 projViewCubeMatrices[6];
};

in vec2 iuv_g[];
in vec3 iwn_g[];
in vec3 iwt_g[];
in vec3 iwb_g[];

out vec2 iuv;    // UVs
out vec3 iwv;    // world-space position
out vec3 iwn;    // world-space normal
out vec3 iwt;    // world-space tangent
out vec3 iwb;    // world-space bitangent

void main()
{
    for (int i = 0; i < 6; ++i)
    {
        gl_Layer = i;

        for (int j = 0; j < 3; ++j)
        {
            // world-space position
            iwv = gl_in[j].gl_Position.xyz / gl_in[j].gl_Position.w;
            // screen-space position
            gl_Position = projViewCubeMatrices[i] * gl_in[j].gl_Position;
            // UVs
            iuv = iuv_g[j];
            // TBN
            iwn = iwn_g[j];
            iwt = iwt_g[j];
            iwb = iwb_g[j];

            EmitVertex();
        }
        EndPrimitive();
    }
}