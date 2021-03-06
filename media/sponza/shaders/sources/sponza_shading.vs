layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 shadowMatrix;

out vec2 iuv;
out vec3 ivv;
out mat3 itbn;
out vec3 wnorm;
out vec4 svv;

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
    // Shadow space transformation
    svv = shadowMatrix * vec4(vertex, 1);

    gl_Position = projectionMatrix * viewMatrix * wv;
}