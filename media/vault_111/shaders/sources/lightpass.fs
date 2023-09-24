#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform sampler2D VwMap;
uniform sampler2D NwMap;

#ifdef USE_SHADOWMAP
uniform sampler2DShadow ShadowMap;
uniform mat4 ShadowMatrix;
#endif

uniform vec3 eye;

#define LITE3D_LIGHT_UNDEFINED          0
#define LITE3D_LIGHT_POINT              1
#define LITE3D_LIGHT_DIRECTIONAL        2
#define LITE3D_LIGHT_SPOT               3

struct lightSource 
{
    int enabled;
    int type;
    vec3 position;
    vec3 diffuse;
    vec3 direction;
    float influenceDistance;
    float attenuationContant;
    float attenuationLinear;
    float attenuationQuadratic;
    float innercone;
    float outercone;
    float radiance;
};

uniform lightSource Light;

in vec2 iuv;

const float PI = 3.14159265359;

// Fresnel equation (Schlick)
vec3 F(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
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
vec3 BRDF(vec3 albedo, float NdotL, float HdotV, float NdotV, float NdotH, vec3 specular, vec3 F0)
{
    float ndf = NDF(NdotH, specular.y);
    float g = G(NdotV, NdotL, specular.y);
    vec3 f = F(HdotV, F0) * specular.x;
    // PBR модель строится на принципе сохранения энергии и по этому энергия поглощенного и отраженного 
    // света в суммме не могут быть больше чем энергия падающего луча от источника света  
    // f - Кофф Френеля по сути определяет отраженную часть света, поэтому kD - Кофф поглащенного света
    // вычисляется просто kD = 1 - f , но с поправкой на металл/диэлектрик. Металл хуже поглощает свет.
    vec3 kD = vec3(1.0) - f;
    kD *= 1.0 - specular.z;
        
    vec3 s = (ndf * g * f) / (4.0 * NdotV * NdotL + 0.0001);
    return kD * albedo / PI + s;
}

vec3 Lx(vec3 albedo, vec3 radiance, vec3 L, vec3 N, vec3 V, vec3 specular, vec3 F0)
{
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    // Уравнение отражения для источника света
    return BRDF(albedo, NdotL, HdotV, NdotV, NdotH, specular, F0) * radiance * NdotL;
}

void main()
{
    if (Light.enabled == 0)
        discard;
    // sampling normal in world space from fullscreen normal map
    vec3 nw = texture(NwMap, iuv).xyz;
    // Non shaded fragment
    if (fiszero(nw))
        discard;

    vec3 lightDirection = normalize(Light.direction);
    float lightDistance = 0.0;
    float attenuationFactor = 1.0;
    // sampling fragment position in world space from fullscreen normal map
    vec3 vw = texture(VwMap, iuv).xyz;
    if (Light.type != LITE3D_LIGHT_DIRECTIONAL)
    {
        /* calculate direction from fragment to light */
        lightDirection = Light.position - vw;
        lightDistance = length(lightDirection);
        lightDirection = normalize(lightDirection);

        if (lightDistance > Light.influenceDistance)
            discard;

        float spotAttenuationFactor = 1.0;
        if (Light.type == LITE3D_LIGHT_SPOT)
        {
            /* calculate spot attenuation */
            float spotAngleRad = acos(dot(-lightDirection, normalize(Light.direction)));
            float spotConeAttenuation = (spotAngleRad * 2.0 - Light.innercone) / (Light.outercone - Light.innercone);
            spotAttenuationFactor = clamp(1.0 - spotConeAttenuation, 0.0, 1.0);
        }

        /* calculate full attenuation */
        attenuationFactor = spotAttenuationFactor / 
            (Light.attenuationContant + 
            Light.attenuationLinear * lightDistance + 
            Light.attenuationQuadratic * lightDistance * lightDistance);
    }

    /* light source full radiance at fragment position */
    vec3 radiance = Light.diffuse * Light.radiance * attenuationFactor;
    // sampling albedo from fullscreen map
    vec3 albedo = texture(Albedo, iuv).xyz;
    // sampling specular parameters from fullscreen map
    vec3 specular = texture(Specular, iuv).xyz;
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.xyz, specular.z);
    // Eye direction to current fragment 
    vec3 eyeDir = normalize(eye - vw);
    
    vec3 lx = Lx(albedo, radiance, lightDirection, nw, eyeDir, specular, F0);
    gl_FragColor = vec4(lx, 1.0);
}