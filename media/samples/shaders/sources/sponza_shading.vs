in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

varying vec2 tcoords;
varying vec3 vnormal;

// common functions
vec4 rtransform(vec4 v1);
mat4 normalMatrix();

void main()
{
	tcoords = texCoordAttr;
	vnormal = normalAttr;
	//normal = normalize(normalMatrix() * vec4(normalAttr, 0.0));
	gl_Position = rtransform(vertexAttr);
}