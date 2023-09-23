#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
uniform float Roughness;
uniform float Specular;
uniform sampler2D AlbedoMask;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    vec4 mask = texture(AlbedoMask, iuv);
    gl_FragColor = vec4(Albedo.rgb, mask.w);
}