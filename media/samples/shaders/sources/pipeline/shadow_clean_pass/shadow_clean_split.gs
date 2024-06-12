layout(triangles) in;
layout(triangle_strip, max_vertices = MAX_SHADOW_SPLIT_VERTICES) out;

layout(std140) uniform ShadowIndex
{
    ivec4 shadowIndex[MAX_SHADOW_LAYERS / 4 + 1];
};

void main()
{
    int count = shadowIndex[0].x;
    for (int i = 1; i <= count; ++i)
    {
        gl_Layer = shadowIndex[i/4][int(mod(i, 4))];

        for (int j = 0; j < 3; ++j)
        {
            gl_Position = gl_in[j].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}