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

void main()
{
    // sampling albedo from fullscreen map
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular parameters from fullscreen map
    vec4 specular = texture(Specular, iuv);
    // sampling normal in world space from fullscreen normal map
    vec3 nw = texture(NwMap, iuv).xyz;
    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(VwMap, iuv).xyz;
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.xyz, specular.z);

    /* calculate direction from fragment to eye */
    vec3 eyeDir = normalize(eye - vw);
    float NdotV = max(dot(nw, eyeDir), 0.0);

    vec3 kS = fresnelSchlickRoughness(NdotV, F0, specular.y) * specular.x; 
    vec3 kD = 1.0 - kS;
    vec3 globalIrradiance = textureLod(Environment, nw, specular.y * 3.0 + 2.0).rgb;
    vec3 ambientAndEmission = (kD * globalIrradiance + albedo.w) * albedo.xyz;

    gl_FragColor = vec4(ambientAndEmission, 1.0);
}