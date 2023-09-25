#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform sampler2D VwMap;
uniform sampler2D NwMap;
uniform samplerCube Environment;

// declaration for UBO
layout(std140) uniform lightSources
{
    vec4 lights[5 * 100];
};

layout(std140) uniform lightIndexes
{
    ivec4 indexes[100];
};

uniform vec3 eye;

in vec2 iuv;

#define M_PI 3.1415926535897932384626433832795
#define FLT_EPSILON 1.192092896e-07F
const float ambientStrength = 0.35;

#define LITE3D_LIGHT_UNDEFINED          0.0
#define LITE3D_LIGHT_POINT              1.0
#define LITE3D_LIGHT_DIRECTIONAL        2.0
#define LITE3D_LIGHT_SPOT               3.0

// Fresnel equation (Schlick)
vec3 FresnelSchlickRoughness(float NdotV, vec3 albedo, vec3 specular)
{
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, specular.z);

    vec3 F = F0 + (max(vec3(1.0 - specular.y), F0) - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
    return F * specular.x;
}

// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
	
    return num / denom;
}

// Geometry function (Schlick-Beckmann, Schlick-GGX)
float GGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

// Geometry function (Smith's)
float G(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GGX(NdotV, roughness);
    float ggx1  = GGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// cook-torrance bidirectional reflective distribution function
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float NdotV, float NdotH, vec3 specular, vec3 F)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    // PBR модель строится на принципе сохранения энергии и по этому энергия поглощенного и отраженного 
    // света в суммме не могут быть больше чем энергия падающего луча от источника света  
    // f - Кофф Френеля по сути определяет отраженную часть света, поэтому kD - Кофф поглащенного света
    // вычисляется просто kD = 1 - f , но с поправкой на металл/диэлектрик. Металл хуже поглощает свет.
    vec3 kD = 1.0 - F;
    kD *= 1.0 - specular.z;
        
    vec3 s = (ndf * g * F) / (4.0 * NdotV * NdotL);
    return kD * albedo / M_PI + s;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, vec3 F, float NdotV)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), FLT_EPSILON);
    float HdotV = max(dot(H, V), FLT_EPSILON);
    float NdotH = max(dot(N, H), FLT_EPSILON);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, NdotL, HdotV, NdotV, NdotH, specular, F) * radiance * NdotL;
}

void main()
{
    // sampling normal in world space from fullscreen normal map
    vec3 nw = texture(NwMap, iuv).xyz;
    // Non shaded fragment
    if (fiszero(nw))
        discard;

    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(VwMap, iuv).xyz;
    // sampling albedo from fullscreen map
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular parameters from fullscreen map
    vec3 specular = texture(Specular, iuv).xyz;
    // Eye direction to current fragment 
    vec3 eyeDir = normalize(eye - vw);
    // Reflect vector for ambient specular
    vec3 R = reflect(eyeDir, nw);
    // NdotV for Fresnel
    float NdotV = max(dot(nw, eyeDir), FLT_EPSILON);
    // Fresnel by Schlick aproxx
    vec3 F = FresnelSchlickRoughness(NdotV, albedo.xyz, specular);

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
            /* block2.w - size */
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

        /* block1.x - diffuse.r */
        /* block1.y - diffuse.g  */
        /* block1.z - diffuse.b */
        /* block1.w - radiance */
        vec4 block1 = lights[index+1];
        /* light source full radiance at fragment position */
        vec3 radiance = block1.rgb * block1.w * attenuationFactor;
        /* L for current lights source */ 
        totalLx += Lx(albedo.rgb, radiance, lightDirection, nw, eyeDir, specular, F, NdotV);
    }

    vec3 kD = 1.0 - F;

    vec3 globalIrradiance = textureLod(Environment, nw, 4).rgb;
    vec3 reflected = textureLod(Environment, R, specular.y * 7.0).rgb * F * ambientStrength;
    vec3 ambient = kD * globalIrradiance * albedo.rgb * ambientStrength;
    vec3 emission = albedo.w * albedo.rgb;

    gl_FragColor = vec4(ambient + reflected + emission + totalLx, 1.0);
}