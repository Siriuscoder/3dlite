layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 5) in mat4 modelMatrix;

out vec2 iuv_g;
out vec3 iwn_g;

void main()
{
    // texture coordinate 
    iuv_g = uv;
    // vertex coordinate in world space 
    gl_Position = modelMatrix * vertex;
    // calculate normal in world space
    iwn_g = normalize(modelMatrix * vec4(normal, 0.0)).xyz;
}