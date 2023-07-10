#include "samples:shaders/sources/common/version.def"

vec3 phong_blinn_single(int type, vec3 lightDir, vec3 eyeDir, vec3 diffuse, 
    vec3 specular, vec3 normal, vec3 spotDirection, vec2 spotFactor, vec3 attenuation, 
    float specularFactor, float wrapAroundFactor, float specPower)
{
    vec3 ldir = normalize(lightDir);
    float ldist = length(lightDir);
    float spotAttenuationFactor = 1.0;
    float attenuationFactor = 1.0;

    // no attenuation for direction light
    if (type != 2)
    {
        if (type == 3)
        {
            /* calculate spot attenuation */
            float spotAngle = acos(dot(-ldir, normalize(spotDirection)));
            spotAttenuationFactor = max((spotFactor.y/2) - spotAngle, 0) / (spotFactor.y/2);
        }

        /* calculate attenuation */
        attenuationFactor = spotAttenuationFactor / (attenuation.x + attenuation.y * ldist + attenuation.z * ldist * ldist);
    }
    else
    {
        // Directional light
        ldir = -normalize(spotDirection);
    }
    
    /* calculate lambertian ratio */
    float lambRatio = max(dot(normal, ldir) + wrapAroundFactor, 0.0) / (1.0 + wrapAroundFactor);
    /* calculate specular ratio */
    float specRatio = pow(max(dot(normal, normalize(ldir + eyeDir)), 0.0), specPower) * specularFactor;

    vec3 linearSpec = specular * specRatio * attenuationFactor;
    /* calculate linear color factor */
    return (diffuse * lambRatio * attenuationFactor) + linearSpec;
}
