#include "samples:shaders/sources/common/common_inc.glsl"

layout(std140) uniform LightSources
{
    LightSource lights[LITE3D_MAX_LIGHT_COUNT];
};

layout(std140) uniform LightIndexes
{
    ivec4 lightsIndexes[LITE3D_MAX_LIGHT_COUNT];
};

/* Shadow compute module */
float Shadow(in LightSource source, in Surface surface, in AngularInfo angular);
/* Illumination compute module */
vec3 BRDF(in Surface surface, in AngularInfo angular);
/* Indirect lighting */
vec3 ComputeIndirect(in Surface surface, in AngularInfo angular);

vec3 ComputeIllumination(in Surface surface)
{
    AngularInfo angular;
    angularInfoInit(angular, surface);
    vec3 directLx = vec3(0.0);

    int count = lightsIndexes[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = lightsIndexes[i/4][int(mod(i, 4))];
        LightSource light = lights[index];
        // Check for the light source is enabled
        if (!hasFlag(light.flags, LITE3D_LIGHT_ENABLED))
            continue;

        // Calc angular info respect to the light source
        angularInfoSetLightSource(angular, surface, light);
        if (angular.isOutside)
            continue;

        // Calc angular dot products respect to the light source
        angularInfoCalcAngles(angular, surface);
        // Calc the light source attenuation
        float attenuationFactor = calcAttenuation(light, angular);
        // Calc shadow, 0 - fully is in shadow, 1 - visible
        float shadowFactor = Shadow(light, surface, angular);
        // Calc the the light source radiance
        vec3 radiance = light.diffuse.rgb * light.radiance * attenuationFactor * shadowFactor * surface.ao;
        // Radiance is too small, consider to skip BRDF calculation
        if (isZero(radiance))
            continue;

        // Calculate BRDF
        directLx += BRDF(surface, angular) * radiance * angular.NdotL;
    }

    // Calculate indirect lighting, ambient, IBL .. 
    vec3 indirectLx = ComputeIndirect(surface, angular);

    return indirectLx + directLx + surface.material.emission.rgb;
}
