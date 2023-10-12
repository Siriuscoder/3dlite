#include "samples:shaders/sources/common/version.def"

uniform vec4 Albedo;
uniform float Roughness;
uniform float Specular;
uniform sampler2D AlbedoMask;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    vec4 mask = texture(AlbedoMask, iuv);
    fragColor = vec4(Albedo.rgb, mask.w);
}