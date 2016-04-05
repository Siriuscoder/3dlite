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
	return transpose(inverse(modelMatrix));
}

vec4 rntransform(vec3 normal)
{
	return normalize(modelMatrix * vec4(normal, 0));
}
