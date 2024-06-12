#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

in vec2 iuv;

// You must implement this methods in you shader to provide data for depth pass
vec4 getAlbedo(vec2 uv);

void main()
{
    // check albedo alpha and discard full transparent fragments
    vec4 albedo = getAlbedo(iuv);
    if (fiszero(albedo.a))
        discard;
}