#version 330

in vec4 vertexAttr;
in vec3 normalAttr;
in vec2 texCoordAttr;

out vec2 tcoords;
out vec3 vnormal;

// common functions
vec4 rtransform(vec4 v1);
mat4 normalMatrix();
vec3 rntransform(vec3 normal);

const vec3 startPosition = vec3(-5000, -5000, 0);
const float offset = 1200;
const int rowCount = 10;

void main()
{
    tcoords = texCoordAttr;
    vnormal = rntransform(normalAttr);
    
    float xOffset = mod(gl_InstanceID, rowCount) * offset;
    float yOffset = int(gl_InstanceID / rowCount) * offset;
    
    gl_Position = rtransform(vertexAttr + vec4(startPosition.x + xOffset, startPosition.y + yOffset, startPosition.z, 0));
}