layout(location = 0) in vec3 vertex;

uniform mat4 modelMatrix;

out vec3 iuv_g;

void main()
{
    // texture coordinate 
    iuv_g = vertex * -1.0;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vec4(vertex, 0.0);
}
