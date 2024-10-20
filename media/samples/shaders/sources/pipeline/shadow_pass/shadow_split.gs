layout(triangles) in;
layout(triangle_strip, max_vertices = LITE3D_SPOT_SHADOW_GS_MAX_VERTICES) out;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[LITE3D_SPOT_SHADOW_MAX_COUNT];
};

layout(std140) uniform ShadowIndex
{
    ivec4 shadowIndex[LITE3D_SPOT_SHADOW_MAX_COUNT / 4 + 1];
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