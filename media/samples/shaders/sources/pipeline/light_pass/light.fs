#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2DArray GBuffer;

in vec2 iuv;
out vec4 fragColor;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);
float getAmbientOcclusion(vec2 uv);

void main()
{
    // sampling normal in world space from fullscreen normal map
    vec4 nw = texture(GBuffer, vec3(iuv, 1));
    // Non shaded fragment
    if (fiszero(nw.xyz))
        discard;

    // sampling fragment position in world space from fullscreen normal map
    vec4 vw = texture(GBuffer, vec3(iuv, 0));
    // sampling albedo from fullscreen map
    vec4 albedo = texture(GBuffer, vec3(iuv, 2));
    // sampling specular parameters from fullscreen map
    vec4 specular = texture(GBuffer, vec3(iuv, 3));
    // sampling AO
    float aoFactor = getAmbientOcclusion(iuv);
    // Emission
    vec3 emission = vec3(nw.w, albedo.w, specular.w);
    // Compute total illumination 
    vec3 total = ComputeIllumination(vw.xyz, nw.xyz, albedo.rgb, emission, specular.xyz, aoFactor, vw.w);

    fragColor = vec4(total, 1.0);
}
