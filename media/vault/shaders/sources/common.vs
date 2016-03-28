#version 150

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 modelviewMatrix;

vec4 rtransform(vec4 v1)
{
	return projectionMatrix * modelviewMatrix * v1;
}

mat4 normalMatrix()
{
	return viewMatrix * transpose(inverse(modelMatrix));
}
