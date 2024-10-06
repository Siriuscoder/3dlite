layout(triangles) in;
layout(triangle_strip, max_vertices = ENV_MULTI_MAX_VERTICES) out;

struct EnvProbeStruct
{
    vec4 position;
    mat4 projView[6];
};

layout(std140) uniform EnvProbes
{
    EnvProbeStruct probes[ENV_PROBES_MAX];
};

layout(std140) uniform EnvProbesIndex
{
    int probesIndex[ENV_PROBES_MAX + 1];
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
                // TBN
                iwn = iwn_g[j];
                iwt = iwt_g[j];
                iwb = iwb_g[j];

                EmitVertex();
            }
            EndPrimitive();
        }
    }
}