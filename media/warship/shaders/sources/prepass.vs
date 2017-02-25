#version 330

in vec3 vertex;
in vec3 normal;
in vec2 uv;
in vec3 tang;
in vec3 binorm;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;


out vec2 iuv;
out vec3 ivv;
out mat3 itbn;

void main()
{
    // texture coordinate 
    iuv = uv;
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vec4(vertex, 1);
    wv = wv / wv.w;
    ivv = wv.xyz;
    // calculate TBN matrix to transform normal from tangent space to world space
    itbn = mat3(normalize(normalMatrix * tang),
        normalize(normalMatrix * binorm),
        normalize(normalMatrix * normal));

    gl_Position = projectionMatrix * viewMatrix * wv;
}