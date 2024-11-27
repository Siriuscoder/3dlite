#include "samples:shaders/sources/common/env_probe_inc.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = LITE3D_ENV_PROBE_GS_MAX_VERTICES) out;

layout(std140) uniform EnvProbesData
{
    EnvironmentProbeStruct probes[LITE3D_ENV_PROBE_MAX];
};

layout(std140) uniform EnvProbesIndex
{
    int probesIndex[LITE3D_ENV_PROBE_MAX + 1];
};

in vec2 iuv_g[];
in vec3 iwn_g[];
flat in int drawID_g[];

out vec2 iuv;           // UVs
out vec3 iwv;           // world-space position
out vec3 iwn;           // world-space normal
flat out int drawID;    // currect chunk index

void main()
{
    int indexCount = probesIndex[0];
    for (int ic = 1; ic <= indexCount; ++ic)
    {
        int index = probesIndex[ic];
        for (int i = 0; i < 6; ++i)
        {
            gl_Layer = (index * 6) + i;

            for (int j = 0; j < 3; ++j)
            {
                // world-space position
                iwv = gl_in[j].gl_Position.xyz / gl_in[j].gl_Position.w;
                // screen-space position
                gl_Position = probes[index].projView[i] * gl_in[j].gl_Position;
                // UVs
                iuv = iuv_g[j];
                // N
                iwn = iwn_g[j];
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
                drawID = drawID_g[j];
#endif
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}