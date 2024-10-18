uniform sampler2D diffuse;
uniform float animcounter; // [0..1]

in vec2 iuv;
in vec3 ivv;
in vec3 wnorm;

layout(location = 0) out vec4 coord;
layout(location = 1) out vec4 norm;
layout(location = 2) out vec4 color;

void main()
{
    // sampling diffuse color 
    vec4 fragDiffuse = vec4(texture(diffuse, vec2(iuv.x, iuv.y + animcounter)).rgb, 1.0);
    coord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    norm = vec4(wnorm, 0);
    color = fragDiffuse;
}