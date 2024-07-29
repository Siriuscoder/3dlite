#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCubeArray EnvironmentProbe;

struct EnvProbeStruct
{
    vec4 position;
    mat4 projView[6];
};

layout(std140) uniform LightProbes
{
    EnvProbeStruct probes[ENV_PROBES_MAX];
};

vec3 ComputeEnvironmentLighting(vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    // Reflect vector from surface
    vec3 R = reflect(-V, N) * CUBE_MAP_UV_SCALE;
    // Fresnel by Schlick aproxx
    vec3 F = fresnelSchlickRoughness(NdotV, albedo, specular);
    // Duffuse irradiance 
 //  vec3 globalIrradiance = texture(IrradianceProbe, N * CUBE_MAP_UV_SCALE).rgb;
///vec3 diffuseEnv = diffuseFactor(F, specular.z) * albedo * globalIrradiance * DIFFUSE_IRRADIANCE_STRENGTH;
    // Specular 
   // float specularLevel = clamp(sqrt(specular.y) * float(SPECULAR_MAX_LOD), float(SPECULAR_MIN_LOD), float(SPECULAR_MAX_LOD));
 //   vec3 specularEnv = textureLod(EnvironmentProbe, R * CUBE_MAP_UV_SCALE, specularLevel).rgb * F * saFactor;

    vec3 diffuseIrradianceLx = vec3(0.0);
    float samplesCount = 0.0;

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    for (int p = 0; p < ENV_PROBES_MAX; ++p)
    {
        float probeDistance = length(P - probes[p].position.xyz);
        float weight = probeDistance * probeDistance;
    
        for (float phi = 0.0; phi < 2.0 * M_PI; phi += DIFFUSE_IRRADIANCE_CONVOLUTION_STEP)
        {
            for (float theta = 0.0; theta < 0.5 * M_PI; theta += DIFFUSE_IRRADIANCE_CONVOLUTION_STEP)
            {
                // spherical to cartesian (in tangent space)
                vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
                // tangent space to world
                vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

                diffuseIrradianceLx += texture(EnvironmentProbe, vec4(sampleVec, p)).rgb * cos(theta) * sin(theta) * weight;
                samplesCount++;
            }
        }
    }

    vec3 kD = diffuseFactor(F, specular.z) * albedo * DIFFUSE_IRRADIANCE_STRENGTH * aoFactor;
    diffuseIrradianceLx = M_PI * diffuseIrradianceLx / samplesCount;
    return diffuseIrradianceLx * kD;
}
