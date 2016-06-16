#ifdef GL_ES
precision mediump float;
#endif

in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;

varying vec2 tcoords;

void main()
{
	tcoords = texCoordAttr;
	gl_Position = projectionMatrix * modelviewMatrix * vertexAttr;
}