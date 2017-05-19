#version 330

in vec3 vertex;
in vec3 normal;
in vec2 uv;
in vec3 tang;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec2 iuv;
out vec3 ivv;
out mat3 itbn;
out vec3 wnorm;

void main()
{
    // texture coordinate 
    iuv = uv;
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vec4(vertex, 1);
    ivv = wv.xyz / wv.w;
    // calculate tangent, normal, binormal in world space
    vec3 worldTang = normalize(normalMatrix * tang);
    wnorm = normalize(normalMatrix * normal);
    // re-orthogonalize T with respect to N
    worldTang = normalize(worldTang - dot(worldTang, wnorm) * wnorm);
    vec3 worldBinorm = cross(wnorm, worldTang);
    // TBN matrix to transform normal from tangent space to world space
    itbn = mat3(worldTang, worldBinorm, wnorm);

    gl_Position = projectionMatrix * viewMatrix * wv;
}