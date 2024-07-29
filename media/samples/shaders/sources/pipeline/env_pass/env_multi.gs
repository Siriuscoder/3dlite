layout(triangles) in;
layout(triangle_strip, max_vertices = ENV_MULTI_MAX_VERTICES) out;

struct EnvProbeStruct
{
    vec4 position;
    mat4 projView[6];
};

layout(std140) uniform LightProbes
{
    EnvProbeStruct probes[ENV_PROBES_MAX];
};

in vec2 iuv_g[];
in vec3 iwn_g[];
in vec3 iwt_g[];
in vec3 iwb_g[];

out vec2 iuv;    // UVs
out vec3 iwv;    // world-space position
out vec3 iwn;
out vec3 iwt;
out vec3 iwb;

void main()
{
    int layerID = 0;
    for (int p = 0; p < ENV_PROBES_MAX; ++p)
    {
        for (int i = 0; i < 6; ++i, ++layerID)
        {
            gl_Layer = layerID;

            for (int j = 0; j < 3; ++j)
            {
                // world-space position
                iwv = gl_in[j].gl_Position.xyz / gl_in[j].gl_Position.w;
                // screen-space position
                gl_Position = probes[p].projView[i] * gl_in[j].gl_Position;
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