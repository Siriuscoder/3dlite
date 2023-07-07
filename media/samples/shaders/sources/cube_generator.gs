#include "samples:shaders/sources/common/version.def"

layout(triangles) in;
layout(triangle_strip, max_vertices=36) out;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

// model matrix
in mat4 wm[3];

out vec2 uv;
out vec3 wn;

// common functions
vec4 rtransform(vec4 v1)
{
    return projectionMatrix * viewMatrix * wm[0] * v1;
}

vec3 rntransform(vec3 normal)
{
    return normalize(wm[0] * vec4(normal, 0.0)).xyz;
}

void EmitPrimitive(vec4 v1, vec4 v2, vec4 v3, vec2 uv1, vec2 uv2, vec2 uv3)
{
    wn = rntransform(cross(v2.xyz - v1.xyz, v3.xyz - v1.xyz));
    uv = uv1;
    gl_Position = rtransform(v1);
    EmitVertex();
    uv = uv2;
    gl_Position = rtransform(v2);
    EmitVertex();
    uv = uv3;
    gl_Position = rtransform(v3);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    vec3 center = gl_in[0].gl_Position.xyz;
    vec3 size = gl_in[1].gl_Position.xyz;

/////////////////////////////////////////////////////////////////////////////////////
    vec4 v1 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    vec4 v2 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    vec4 v3 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    v2 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    v3 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    v3 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y+(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    v2 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    v3 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0));
/////////////////////////////////////////////////////////////////////////////////////
    v1 = vec4(center.x-(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v2 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z+(size.z/2), 1.0);
    v3 = vec4(center.x+(size.x/2), center.y-(size.y/2), center.z-(size.z/2), 1.0);
    EmitPrimitive(v3, v2, v1, vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0));
}