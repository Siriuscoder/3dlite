#include "samples:shaders/sources/common/utils_inc.glsl"

#define LITE3D_LIGHT_UNDEFINED          0.0
#define LITE3D_LIGHT_POINT              1.0
#define LITE3D_LIGHT_DIRECTIONAL        2.0
#define LITE3D_LIGHT_SPOT               3.0

layout(std140) uniform LightSources
{
    vec4 lights[LITE3D_MAX_LIGHT_COUNT];
};

layout(std140) uniform LightIndexes
{
    ivec4 indexes[LITE3D_MAX_LIGHT_COUNT];
};

uniform vec3 Eye;

/* Shadow compute module */
float ShadowVisibility(float shadowIndex, vec3 P, vec3 N, vec3 L);
/* Illumination compute module */
vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, float NdotV);
/* Indirect lighting */
vec3 ComputeIndirect(vec3 P, vec3 V, vec3 N, float NdotV, vec3 albedo, vec3 specular, float edF, float esF);

vec3 ComputeIllumination(vec3 P, vec3 N, vec3 albedo, vec3 emission, vec3 specular, float aoF, 
    float esF)
{
    // Eye direction to current fragment 
    vec3 V = normalize(Eye - P);
    // HdotV
    float NdotV = doubleSidedNdotV(N, V);
    vec3 directLx = vec3(0.0);

    int count = indexes[0].x;
    for (int i = 1; i <= count; i++)
    {
        int index = indexes[i/4][int(mod(i, 4))] * 5;

        /* block0.x - type */
        /* block0.y - enabled */
        /* block0.z - influence distance */
        /* block0.w - influence min radiance */
        vec4 block0 = lights[index];
        if (isZero(block0.y))
            continue;

        /* block3.x - direction.x */
        /* block3.y - direction.y */
        /* block3.z - direction.z */
        /* spot directional */
        /* take effect with spot and directional light */
        vec4 block3 = lights[index+3];

        vec3 L = block3.xyz;
        float attenuationFactor = 1.0;
        if (!isNear(block0.x, LITE3D_LIGHT_DIRECTIONAL))
        {
            /* block2.x - position.x */
            /* block2.y - position.y */
            /* block2.z - position.z */
            /* block2.w - user index */
            /* calculate direction from fragment to light */
            L = lights[index+2].xyz - P;
            float lightDistance = length(L);
            /* skip untouchable light source */
            if (lightDistance > block0.z)
                continue;

            L = normalize(L);
            /* block4.z - angle inner cone */
            /* block4.w - angle outer cone */
            vec4 block4 = lights[index+4];

            float spotAttenuationFactor = 1.0;
            if (isNear(block0.x, LITE3D_LIGHT_SPOT))
            {
                /* calculate spot attenuation */
                float spotAngleRad = acos(dot(-L, normalize(block3.xyz)));
                float spotConeAttenuation = (spotAngleRad * 2.0 - block4.z) / (block4.w - block4.z);
                spotAttenuationFactor = clamp(1.0 - spotConeAttenuation, 0.0, 1.0);
            }

            float edgeFallof = (block0.z - clamp(lightDistance, block0.z * 0.9, block0.z)) / (block0.z * 0.1);
            /* attenuation factor */
            /* block3.w - attenuation constant */
            /* block4.x - attenuation linear */
            /* block4.y - attenuation quadratic */
            /* calculate full attenuation */
            attenuationFactor = spotAttenuationFactor * edgeFallof / 
                (block3.w + block4.x * lightDistance + block4.y * lightDistance * lightDistance);
        }
        else
        {
            L *= -1.0;
        }
        /* User Index, at this implementation is shadow index */
        float shadowVisibility = ShadowVisibility(lights[index+2].w, P, N, L);
        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        vec4 block1 = lights[index+1];
        /* light source full radiance at fragment position */
        vec3 radiance = block1.rgb * block1.w * attenuationFactor * shadowVisibility * aoF;
        /* Radiance too small, do not take this light source in account */ 
        if (isZero(radiance))
            continue;
        /* L for current lights source */ 
        directLx += Lx(albedo, radiance, L, N, V, specular, NdotV);
    }

    vec3 indirectLx = ComputeIndirect(P, V, N, NdotV, albedo, specular, 
        ENV_DIFFUSE_STRENGTH, esF) * aoF;

    return indirectLx + directLx + emission;
}
