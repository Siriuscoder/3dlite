#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Normal;
uniform sampler2D Specular;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

void main()
{
    vec4 albedo = texture(Albedo, iuv);
    fragColor = vec4(albedo.rgb, 1.0);
}