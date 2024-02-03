#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D Albedo;

in vec2 iuv;

void main()
{
    // check albedo alpha and discard full transparent fragments
    vec4 albedo = texture(Albedo, iuv);
    if (fiszero(albedo.a))
        discard;
}