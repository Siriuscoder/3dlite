layout(location = 0) in vec2 vertex;

uniform mat4 screenMatrix;

out vec2 iuv;

void main()
{
    iuv = vertex;
    gl_Position = screenMatrix * vec4(vertex.xy, 0.0, 1.0);
}