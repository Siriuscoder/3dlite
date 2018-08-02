#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 worldPos[3];
in vec2 uv[3];

out vec2 tcoords;
out vec3 vnormal;

void main()
{
    vec3 n = cross(worldPos[1].xyz - worldPos[0].xyz, worldPos[2].xyz - worldPos[0].xyz);
    for(int i = 0; i < gl_in.length(); i++)
    {
        gl_Position = gl_in[i].gl_Position;
    
        tcoords = uv[i];
        vnormal = n;
    
        EmitVertex();
    }
    
    EndPrimitive();
}