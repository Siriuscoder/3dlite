#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform sampler2D VwMap;
uniform sampler2D NwMap;
uniform samplerCube Environment;

uniform vec3 eye;

in vec2 iuv;

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

const float ambientStrength = 0.65;

void main()
{
    // sampling normal in world space from fullscreen normal map
    vec3 nw = texture(NwMap, iuv).xyz;
    // Non shaded fragment
    if (fiszero(nw))
        discard;

    // sampling albedo from fullscreen map
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular parameters from fullscreen map
    vec4 specular = texture(Specular, iuv);
    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(VwMap, iuv).xyz;
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.xyz, specular.z);

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - vw);
    float NdotV = max(dot(nw, eyeDir), 0.0);
    vec3 R = reflect(eyeDir, nw);

    vec3 kS = fresnelSchlickRoughness(NdotV, F0, specular.y) * specular.x; 
    vec3 kD = 1.0 - kS;
    vec3 globalIrradiance = textureLod(Environment, nw, 4).rgb;
    vec3 reflected = textureLod(Environment, R, specular.y * 7.0).rgb * kS * ambientStrength;
    vec3 ambient = kD * globalIrradiance * albedo.xyz * ambientStrength;
    vec3 emission = albedo.w * albedo.xyz;

    gl_FragColor = vec4(ambient + reflected + emission, 1.0);
}