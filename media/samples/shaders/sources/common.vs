#version 330

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

vec4 wtransform(vec4 v1)
{
    return modelMatrix * v1;
}

vec4 rtransform(vec4 v1)
{
    return projectionMatrix * viewMatrix * wtransform(v1);
}

vec3 rntransform(vec3 normal)
{
    return normalize(modelMatrix * vec4(normal, 0.0)).xyz;
}
