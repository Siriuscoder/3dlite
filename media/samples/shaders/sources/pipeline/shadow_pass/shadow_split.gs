layout(triangles) in;
layout(triangle_strip, max_vertices = LITE3D_SPOT_SHADOW_GS_MAX_VERTICES) out;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowTransform[LITE3D_SPOT_SHADOW_MAX_COUNT];   
};

layout(std140) uniform ShadowIndex
{
    ivec4 shadowIndex[LITE3D_SPOT_SHADOW_MAX_COUNT / 4 + 1];
};

in vec2 iuv_g[];
flat in int drawID_g[];

out vec2 iuv;
flat out int drawID;

void main()
{
    int count = shadowIndex[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = shadowIndex[i/4][int(mod(i, 4))];
        gl_Layer = index;

        for (int j = 0; j < 3; ++j)
        {
            gl_Position = shadowTransform[index] * gl_in[j].gl_Position;
            iuv = iuv_g[j];
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
            drawID = drawID_g[j];
#endif
            EmitVertex();
        }
        EndPrimitive();
    }
}