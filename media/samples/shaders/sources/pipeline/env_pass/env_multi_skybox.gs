#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 72) out; // 4 probe max

layout(std140) uniform EnvProbesData
{
    EnvironmentProbeStruct probes[ENV_PROBES_MAX];
};

layout(std140) uniform EnvProbesIndex
{
    int probesIndex[ENV_PROBES_MAX + 1];
};


in vec3 iuv_g[];
out vec3 iuv;    // UVs

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
                // screen-space position
                vec4 ss = probes[index].projView[i] * gl_in[j].gl_Position;
                gl_Position = ss.xyww;
                // UVs
                iuv = iuv_g[j];
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}