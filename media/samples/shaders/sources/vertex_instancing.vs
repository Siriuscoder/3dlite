#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec4 v;
layout(location = 1) in vec3 n;
layout(location = 2) in vec2 tc;

out vec2 uv;
out vec3 wn;

// common functions
vec4 rtransform(vec4 v1);
vec3 rntransform(vec3 normal);

const vec3 startPosition = vec3(-5000, -5000, 0);
const float offset = 1200;
const int rowCount = 10;

void main()
{
    uv = tc;
    wn = rntransform(n);
    
    float xOffset = mod(gl_InstanceID, rowCount) * offset;
    float yOffset = int(gl_InstanceID / rowCount) * offset;
    
    gl_Position = rtransform(v + vec4(startPosition.x + xOffset, startPosition.y + yOffset, startPosition.z, 0));
}