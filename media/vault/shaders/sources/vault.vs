in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

varying vec3 vnormal;
varying vec2 vtcoords;

// common functions
vec4 rtransform(vec4 v1);
mat4 normalMatrix();

void main()
{
	vtcoords = texCoordAttr;
	vnormal = normalize(normalAttr);
	gl_Position = rtransform(vertexAttr);
}