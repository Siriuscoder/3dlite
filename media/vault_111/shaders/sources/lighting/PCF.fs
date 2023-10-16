#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArrayShadow ShadowMaps;

layout(std140) uniform ShadowMatrix
{
    mat4 shadowMat[4];
};

const float shadowBias = 0.00001;

float PCF(float shadowIndex, vec3 vw)
{
    // Do not cast shadows
    if (shadowIndex < 0.0)
        return 1.0;

    // Shadow space NDC coorts of current fragment
    vec4 sv = shadowMat[int(shadowIndex)] * vec4(vw, 1.0);
    // transform the NDC coordinates to the range [0,1]
    sv = (sv.xyzw / sv.w) * 0.5 + 0.5;
    // Z clip 
    if (sv.z > 1.0)
        return 0.0;

    float result = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMaps, 0).xy;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 shift = sv.xy + (vec2(x, y) * texelSize);
            if (shift.x < 0.0 || shift.x > 1.0 || shift.y < 0.0 || shift.y > 1.0)
                continue;

            result += texture(ShadowMaps, vec4(shift, shadowIndex, sv.z - shadowBias));
        }
    }

    result /= 9.0;
    return result;
}
