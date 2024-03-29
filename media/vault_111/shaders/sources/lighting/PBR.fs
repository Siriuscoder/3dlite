#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

#define MAX_LIGHTS  200 // 16kb storage needed

const float specularStrength = 0.14;
const float diffuseStrength = 0.05;

#define LITE3D_LIGHT_UNDEFINED          0.0
#define LITE3D_LIGHT_POINT              1.0
#define LITE3D_LIGHT_DIRECTIONAL        2.0
#define LITE3D_LIGHT_SPOT               3.0

uniform samplerCube Environment;

layout(std140) uniform lightSources
{
    vec4 lights[5 * MAX_LIGHTS];
};

layout(std140) uniform lightIndexes
{
    ivec4 indexes[MAX_LIGHTS];
};

uniform vec3 eye;

/* Shadow compute module */
float PCF(float shadowIndex, vec3 vw);
/* Illumination compute module */
vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV);
/* Fresnel equation (Schlick) */
vec3 FresnelSchlickRoughness(float NdotV, vec3 albedo, vec3 specular);

vec3 ComputeIllumination(vec3 vw, vec3 nw, vec3 albedo, vec3 emission, vec3 specular, float aoFactor)
{
    // Eye direction to current fragment 
    vec3 eyeDir = normalize(eye - vw);
    // Reflect vector for ambient specular
    vec3 R = reflect(eyeDir, nw);
    // HdotV for Fresnel
    float NdotV = max(dot(nw, eyeDir), FLT_EPSILON);
    // Fresnel by Schlick aproxx
    vec3 F = FresnelSchlickRoughness(NdotV, albedo, specular);

    vec3 totalLx = vec3(0.0);

    int count = indexes[0].x;
    for (int i = 1; i <= count; i++)
    {
        int index = indexes[i/4][int(mod(i, 4))] * 5;

        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        vec4 block0 = lights[index];
        if (fiszero(block0.y))
            continue;

        /* block3.x - direction.x */
        /* block3.y - direction.y */
        /* block3.z - direction.z */
        /* spot directional */
        /* take effect with spot and directional light */
        vec4 block3 = lights[index+3];

        vec3 lightDirection = block3.xyz;
        float attenuationFactor = 1.0;
        if (!fnear(block0.x, LITE3D_LIGHT_DIRECTIONAL))
        {
            /* block2.x - position.x */
            /* block2.y - position.y */
            /* block2.z - position.z */
            /* block2.w - user index */
            /* calculate direction from fragment to light */
            lightDirection = lights[index+2].xyz - vw;
            float lightDistance = length(lightDirection);
            lightDirection = normalize(lightDirection);
            /* skip untouchable light source */
            if (lightDistance > block0.z)
                continue;

            /* block4.z - angle inner cone */
            /* block4.w - angle outer cone */
            vec4 block4 = lights[index+4];

            float spotAttenuationFactor = 1.0;
            if (fnear(block0.x, LITE3D_LIGHT_SPOT))
            {
                /* calculate spot attenuation */
                float spotAngleRad = acos(dot(-lightDirection, normalize(block3.xyz)));
                float spotConeAttenuation = (spotAngleRad * 2.0 - block4.z) / (block4.w - block4.z);
                spotAttenuationFactor = clamp(1.0 - spotConeAttenuation, 0.0, 1.0);
            }

            /* attenuation factor */
            /* block3.w - attenuation constant */
            /* block4.x - attenuation linear */
            /* block4.y - attenuation quadratic */
            /* calculate full attenuation */
            attenuationFactor = spotAttenuationFactor / 
                (block3.w + block4.x * lightDistance + block4.y * lightDistance * lightDistance);
        }
        /* User Index, at this implementation is shadow index */
        float shadowless = PCF(lights[index+2].w, vw);
        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        vec4 block1 = lights[index+1];
        /* light source full radiance at fragment position */
        vec3 radiance = block1.rgb * block1.w * attenuationFactor * shadowless * aoFactor;
        /* Radiance too small, do not take this light source in account */ 
        if (all(lessThan(radiance, vec3(0.0001))))
            continue;
        /* L for current lights source */ 
        totalLx += Lx(albedo, radiance, lightDirection, nw, eyeDir, specular, NdotV);
    }

    vec3 kD = 1.0 - F;
    kD *= 1.0 - specular.z;

    vec3 globalIrradiance = textureLod(Environment, nw, 4).rgb;
    vec3 specularAmbient = textureLod(Environment, R, specular.y * 7.0).rgb * F * specularStrength;
    vec3 diffuseAmbient = kD * globalIrradiance * albedo * diffuseStrength;
    vec3 totalAmbient = (diffuseAmbient + specularAmbient) * aoFactor;

    return totalAmbient + totalLx + emission;
}
