in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

varying vec2 tcoords;
varying vec3 vnormal;

// common functions
vec4 rtransform(vec4 v1);
vec3 rntransform(vec3 normal);

void main()
{
	tcoords = texCoordAttr;
	vnormal = rntransform(normalAttr);
	gl_Position = rtransform(vertexAttr);
}