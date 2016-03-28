in vec4 vertexAttr;
// common functions
vec4 rtransform(vec4 v1);
mat4 normalMatrix();

void main()
{
	gl_Position = rtransform(vertexAttr);
}