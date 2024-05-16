#include "samples:shaders/sources/common/version.def"

uniform sampler2D Normal;

vec3 GetFixedWorldNormal(mat3 tbn, vec2 iuv)
{
    // sampling normal from normal map with swap XY
    vec3 nt = texture(Normal, iuv).rgb;
    // put normal in [-1,1] range in tangent space
    nt = 2.0 * clamp(nt, 0.0, 1.0) - 1.0;
    // trasform normal to world space using common TBN
    return normalize(tbn * nt);
}
