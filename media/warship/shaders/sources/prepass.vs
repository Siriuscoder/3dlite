#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tang;
layout(location = 4) in mat4 modelMatrix;

uniform mat4 projViewMatrix;

out vec2 iuv;
out vec3 ivv;
out mat3 itbn;

void main()
{
    // texture coordinate 
    iuv = uv;
    // vertex coordinate in world space 
    vec4 wv = modelMatrix * vec4(vertex, 1);
    ivv = wv.xyz / wv.w;
    // calculate tangent, normal, binormal in world space
    vec3 worldTang = normalize(modelMatrix * vec4(tang, 0)).xyz;
	vec3 worldNorm = normalize(modelMatrix * vec4(normal, 0)).xyz;
	// re-orthogonalize T with respect to N
	worldTang = normalize(worldTang - dot(worldTang, worldNorm) * worldNorm);
	vec3 worldBinorm = cross(worldNorm, worldTang);
	// TBN matrix to transform normal from tangent space to world space
    itbn = mat3(worldTang, worldBinorm, worldNorm);
    
    gl_Position = projViewMatrix * wv;
}