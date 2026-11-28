#ifdef LITE3D_USE_AREA_LIGHTS
#include "samples:shaders/sources/pbr/LTC.glsl"
#endif

/* Shadow compute module */
float Shadow(in LightSource source, in Surface surface, in AngularInfo angular);
/* BRDF */
SufraceLighting BRDF(in Surface surface, in AngularInfo angular);

float MicrowShadowing(in Surface surface, float NdotL)
{
    float aperture    = 2.0 * surface.ao;
    float shadow = clamp(NdotL + aperture - 1.0, 0.0, 1.0);

    // Suppress microshadowing on smooth metallic surfaces (mirrors)
    float mirrorWeight = surface.material.metallic * 
        (1.0 - smoothstep(0.1, 0.3, surface.material.roughness));
    
    return mix(shadow, 1.0, mirrorWeight);
}

// A local diffuse approximation, not a volumetric thickness simulation. The wrap
// integrates to one over incident directions: the (1 + wrap)^2 denominator keeps
// broadening the lobe from manufacturing light. The caller blends this with the
// ordinary diffuse lobe using the material's scattering fraction.
float SubsurfaceScattering(vec3 normal, vec3 lightDir)
{
    const float wrap = 0.35;
    const float NdotL = dot(normal, lightDir);

    return max(NdotL + wrap, 0.0) / ((1.0 + wrap) * (1.0 + wrap) * M_PI);
}

SufraceLighting EvaluateDirectLight(in LightSource light, in Surface surface, in AngularInfo angular)
{
    SufraceLighting sl = SufraceLighting(vec3(0.0), vec3(0.0), vec3(0.0));

    // Check for the light source is enabled
    if (!hasFlag(light.flags, LITE3D_LIGHT_ENABLED))
        return sl;

    // Calc angular info respect to the light source
    angularInfoSetLightSource(angular, surface, light);
    if (angular.isOutside)
        return sl;

    // Calc angular dot products respect to the light source
    angularInfoCalcAngles(angular, surface);
    
    // Calc the light source attenuation
    float attenuationFactor = calcAttenuation(light, angular);
    
    // Calc shadow, 0 - covered by shadow, 1 - visible
    float shadowVisibility = Shadow(light, surface, angular);

    // Calc microshadow/AO
    float microshadow = MicrowShadowing(surface, angular.NdotL);

    // Calc the the light source radiance
    vec3 radiance = light.diffuse.rgb * light.radiance * attenuationFactor * shadowVisibility * microshadow;

    // Radiance is too small, consider to skip BRDF calculation
    if (isZero(radiance))
        return sl;

#ifdef LITE3D_USE_AREA_LIGHTS
    if (hasFlag(light.flags, LITE3D_LIGHT_RECT_AREA) ||
        hasFlag(light.flags, LITE3D_LIGHT_DISK_AREA))
    {
        sl = LTC(light, surface, angular);
    }
    else
#endif
    {
        // Calculate BRDF
        sl = BRDF(surface, angular);
        sl.diffuse *= angular.NdotL;
        sl.specular *= angular.NdotL;
    }

    float sss = clamp(surface.material.subSurfaceScattering, 0.0, 1.0);
    if (sss > FLT_EPSILON)
    {
        sl.diffuse = mix(sl.diffuse, vec3(SubsurfaceScattering(surface.normal, angular.lightDir)), sss);
    }

    if (surface.material.sheen > FLT_EPSILON)
    {
        sl.specular += Sheen(surface.material, angular) * angular.NdotL;
        // energy - sheen absorbs some diffuse
        sl.diffuseEnergy *= 1.0 - surface.material.sheen * 0.5;
    }

    // reduce deffuse for transparent surfaces
    sl.diffuse *= radiance * sl.diffuseEnergy * surface.material.alpha;
    sl.specular *= radiance;

    return sl;
}
