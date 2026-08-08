#include "samples:shaders/sources/common/common_inc.glsl"

out vec4 fragColor;

uniform mat4 projViewMatrix;
uniform sampler2D OpaqueCombined;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

const vec2 poissonDisk8[8] = vec2[]
(
    vec2(-0.326212, -0.405810),
    vec2(-0.840144, -0.073580),
    vec2(-0.695914,  0.457137),
    vec2(-0.203345,  0.620716),
    vec2( 0.962340, -0.194983),
    vec2( 0.473434, -0.480026),
    vec2( 0.519456,  0.767022),
    vec2( 0.185461, -0.893124)
);

vec3 ComputeIllumination(in Surface surface, in AngularInfo angular);

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);
    // Compute refraction offset 
    AngularInfo angular;
    angularInfoInit(angular, surface);

    // Пока что стекло
    float thickness = 10.0;
    float ior = 1.5;
    float eta = 1.0 / ior;
    // Вектор преломления в мировых координатах
    vec3 T = refract(-angular.viewDir, surface.normal, eta);
    // Мировая координата выхода преломленного луча из поверхности, толщина поверхности задается thickness
    vec3 Pexit = surface.wv + T * thickness;
    // Вычисляем смещение в экранных координатах
    vec4 clip0 = projViewMatrix * vec4(surface.wv, 1.0);
    vec4 clip1 = projViewMatrix * vec4(Pexit, 1.0);
    // Перспективное деление и нормализация
    vec2 uv0 = clip0.xy / clip0.w * 0.5 + 0.5;
    vec2 uv1 = clip1.xy / clip1.w * 0.5 + 0.5;
    // Наконец то смещение получено
    vec2 offset = uv1 - uv0;
    // Получаем цвет из текстуры экрана по смещенным координатам
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(OpaqueCombined, 0));
    vec2 refractUV = uv + offset;
    // Замазываем артефакты возникающие при сильном преломлении по краям, когда преломленный луч выходит за пределами экрана
    float edgeFade = fadeScreenEdge(refractUV);
    refractUV = mix(uv, refractUV, edgeFade);
    // Получаем цвет того что за стеклом c учетом шероховатости стекла
    float refractionBlurScale = 0.03;
    float radius = surface.material.roughness * refractionBlurScale;

    vec3 refracted = vec3(0.0);
    for(int i = 0; i < 8; i++)
    {
        vec2 sampleUV = refractUV + poissonDisk8[i] * radius;
        refracted += texture(OpaqueCombined, sampleUV).rgb;
    }

    refracted /= 8.0;
    // Compute total illumination 
    vec3 reflected = ComputeIllumination(surface, angular); 
    // mix colors via alpha
    fragColor = vec4(mix(refracted, reflected, surface.material.alpha), 1.0);
}
