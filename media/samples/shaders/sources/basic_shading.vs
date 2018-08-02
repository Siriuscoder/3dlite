#version 330

in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

out vec2 tcoords;
out vec3 vnormal;

// common functions
vec4 rtransform(vec4 v1);
vec3 rntransform(vec3 normal);

void main()
{
	tcoords = texCoordAttr;
	vnormal = rntransform(normalAttr);
	gl_Position = rtransform(vertexAttr);
}