layout(location = 0) in vec2 ivertex;

uniform mat4 screenMatrix;

out vec2 iuv;

void main()
{
    iuv = ivertex;
    gl_Position = screenMatrix * vec4(ivertex.xy, 0.0, 1.0);
}