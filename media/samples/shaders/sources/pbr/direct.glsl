#ifdef LITE3D_USE_AREA_LIGHTS
#include "samples:shaders/sources/pbr/LTC.glsl"
#endif

/* Shadow compute module */
float Shadow(in LightSource source, in Surface surface, in AngularInfo angular);
/* BRDF */
SufraceLighting BRDF(in Surface surface, in AngularInfo angular);
// chan 2018, material advances in call of duty wwii
// canonical paper form, the prior rsqrt(1 - visibility) aperture exploded for visibility close
// to one and the n_dot_l multiplication then crushed grazing angles to near zero with even a
// tiny amount of ssao occlusion, which is what made flat ground at sunset go almost black the
// moment gtao bent normals were enabled, the canonical form below stays at one for visibility
// equal to one, only bites near the terminator, and collapses to zero only when visibility and
// n_dot_l vanish together, matching the curves in the cod wwii slides
float MicrowShadowingCOD(float NdotL, float visibility)
{
    float aperture    = 2.0 * visibility;
    float microShadow = clamp(NdotL + aperture - 1.0, 0.0, 1.0);
    return microShadow * microShadow;
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

SufraceLighting EvaluateDirect(in LightSource light, inout Surface surface, inout AngularInfo angular)
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
    if (isZero(attenuationFactor))
        return sl;
    
    // Calc shadow, 0 - covered by shadow, 1 - visible
    float shadowVisibility = Shadow(light, surface, angular);
    if (isZero(shadowVisibility))
        return sl;

    // Calc microshadow/AO 
    float microShadow = MicrowShadowingCOD(angular.NdotL, surface.ao);

    // Calc the the light source radiance
    vec3 radiance = light.diffuse.rgb * light.radiance * attenuationFactor * shadowVisibility * microShadow;

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

    if (surface.material.subSurfaceScattering > FLT_EPSILON)
    {
        sl.diffuse += SubsurfaceScattering(surface.normal, angular.lightDir) * surface.material.subSurfaceScattering;
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
