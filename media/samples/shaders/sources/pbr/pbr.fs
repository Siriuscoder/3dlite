#include "samples:shaders/sources/common/common_inc.glsl"

layout(std140) uniform LightSources
{
    LightSource lights[LITE3D_MAX_LIGHT_COUNT];
};

layout(std140) uniform LightIndexes
{
    ivec4 indexes[LITE3D_MAX_LIGHT_COUNT];
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

    int count = indexes[0].x;
    for (int i = 1; i <= count; ++i)
    {
        int index = indexes[i/4][int(mod(i, 4))];

        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        LightSource light = lights[index];
        if (!hasFlag(light.flags, LITE3D_LIGHT_ENABLED))
            continue;

        angularInfoSetLightSource(angular, surface, light);
        if (angular.isOutside)
            continue;

        float attenuationFactor = CalcAttenuation(light, angular);
        float shadowFactor = Shadow(light, surface, angular);
        /* light source full radiance at fragment position */
        vec3 radiance = light.diffuse.rgb * light.radiance * attenuationFactor * shadowFactor * surface.ao;
        /* Radiance too small, do not take this light source in account */ 
        if (isZero(radiance))
            continue;

        angularInfoCalcAngles(angular, surface);
        /* L for current lights source */ 
        directLx += BRDF(surface, angular) * radiance * angular.NdotL;
    }

    vec3 indirectLx = ComputeIndirect(surface, angular);

    return indirectLx + directLx + emission;
}
