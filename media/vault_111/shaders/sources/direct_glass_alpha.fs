#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform float SpecularAmbientFactor;

out vec4 fragColor;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor, 
    float saFactor);
vec3 GetFixedWorldNormal(mat3 itbn, vec2 iuv);
vec3 GetSpecular(vec2 iuv);

void main()
{
    /* sampling albedo with alpha */
    vec4 albedo = texture(Albedo, iuv);
    // sampling normal and convert to world space
    vec3 nw = GetFixedWorldNormal(itbn, iuv);
    // sampling specular PBR parameters
    vec3 specular = GetSpecular(iuv);
    // Compute total illumination 
    vec3 total = ComputeIllumination(ivv, nw, albedo.rgb, vec3(0.0), vec3(specular.x, specular.y / 10.0, specular.z), 1.0,
        SpecularAmbientFactor);

    float alpha = mix(albedo.a, 1.0, specular.y);
    fragColor = vec4(total, alpha);
}