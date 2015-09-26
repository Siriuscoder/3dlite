in vec4 vertexAttr;
in vec2 texCoordAttr;

varying vec2 tcoords;

// common functions
vec4 rtransform(vec4 v1);
mat4 normalMatrix();

void main()
{
	tcoords = texCoordAttr;
	gl_Position = rtransform(vertexAttr);
}