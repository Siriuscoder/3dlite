#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArray PreparedRenderData;

in vec2 iuv;
out vec4 fragColor;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 specular, float emissionStrength);

void main()
{
    // sampling normal in world space from fullscreen normal map
    vec3 nw = texture(PreparedRenderData, vec3(iuv, 1)).xyz;
    // Non shaded fragment
    if (fiszero(nw))
        discard;

    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(PreparedRenderData, vec3(iuv, 0)).xyz;
    // sampling albedo from fullscreen map
    vec4 albedo = texture(PreparedRenderData, vec3(iuv, 2));
    // sampling specular parameters from fullscreen map
    vec3 specular = texture(PreparedRenderData, vec3(iuv, 3)).xyz;
    // Compute total illumination 
    vec3 total = ComputeIllumination(vw, nw, albedo.rgb, specular, albedo.w);

    fragColor = vec4(total, 1.0);
}
