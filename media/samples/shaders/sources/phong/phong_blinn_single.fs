#include "samples:shaders/sources/common/common_inc.glsl"

vec3 phong_blinn_single(vec3 lightDir, vec3 eyeDir, vec3 normal, in LightSource source,
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec)
{
    vec3 ldir = vec3(0.0);
    float attenuationFactor = 1.0;

    // no attenuation for direction light
    if (!hasFlag(source.flags, LITE3D_LIGHT_DIRECTIONAL))
    {
        ldir = normalize(lightDir);
        float ldist = length(lightDir);
        float spotAttenuationFactor = 1.0;

        if (hasFlag(source.flags, LITE3D_LIGHT_SPOT))
        {
            /* calculate spot attenuation */
            float spotAngle = acos(dot(-ldir, normalize(source.direction.xyz)));
            spotAttenuationFactor = max((source.outerCone / 2.0) - spotAngle, 0.0) / (source.outerCone / 2.0);
        }

        /* calculate attenuation */
        attenuationFactor = spotAttenuationFactor / (source.attenuationConstant + 
            source.attenuationLinear * ldist + 
            source.attenuationQuadratic * ldist * ldist);
    }
    else
    {
        // Directional light
        ldir = -normalize(source.direction.xyz);
    }
    
    /* calculate lambertian ratio */
    float lambRatio = max(dot(normal, ldir) + wrapAroundFactor, 0.0) / (1.0 + wrapAroundFactor);
    /* calculate specular ratio */
    float specRatio = pow(max(dot(normal, normalize(ldir + eyeDir)), 0.0), specPower) * specularFactor;

    linearSpec = source.diffuse.rgb * specRatio * attenuationFactor;
    /* calculate linear color factor */
    return (source.diffuse.rgb * lambRatio * attenuationFactor) + linearSpec;
}
