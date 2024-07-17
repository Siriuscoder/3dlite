#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCube IrradianceMap;
uniform samplerCube SpecularMap;

vec3 ComputeEnvironmentLighting(vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float aoFactor, float saFactor)
{
    return vec3(0.0);
}
