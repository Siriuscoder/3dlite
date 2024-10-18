layout(location = 0) in vec4 vertex;
layout(location = 2) in vec2 uv;

uniform mat4 modelMatrix;
out vec2 vsUV;

void main()
{
    vsUV = uv;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vertex;
}