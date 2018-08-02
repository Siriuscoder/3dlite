#version 330

in vec4 vertexAttr;
in vec2 texCoordAttr;

out vec2 tcoords;

// common functions
vec4 rtransform(vec4 v1);

void main()
{
	tcoords = texCoordAttr;
	gl_Position = rtransform(vertexAttr);
}