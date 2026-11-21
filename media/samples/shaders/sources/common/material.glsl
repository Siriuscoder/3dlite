#if defined(LITE3D_FRAGMENT_SHADER) || defined(LITE3D_COMPUTE_SHADER)

uniform sampler2DArray GBuffer;

vec4 getAlbedo(vec2 uv);
vec3 getEmission(vec2 uv);
vec3 getNormal(vec2 uv, mat3 tbn);
vec3 getSpecular(vec2 uv);
float getAmbientOcclusion(vec2 uv);
float getSpecularAmbient(vec2 uv);
float getIOR(vec2 uv); 
float getSheen(vec2 uv); 

Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb)
{
    Surface surface;
    surface.materialIndex = 0u;
    surface.uv = uv;
    surface.wv = wv;
    surface.ao = getAmbientOcclusion(uv);

    if (!isZero(wt) && !isZero(wb))
    {
        mat3 tbn = TBN(wn, wt, wb);
        surface.normal = getNormal(uv, tbn);
        surface.geoNormal = tbn[2];
        surface.material.flags |= LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT;
    }
    else if (!isZero(wt))
    {
        mat3 tbn = TBN(wn, wt);
        surface.normal = getNormal(uv, tbn);
        surface.geoNormal = tbn[2];
        surface.material.flags |= LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT;
    }
    else 
    {
        surface.normal = normalize(wn);
        surface.geoNormal = surface.normal;
    }

    surface.material.albedo = getAlbedo(uv);
    surface.material.alpha = surface.material.albedo.a;
    surface.material.emission = vec4(getEmission(uv), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.F0 = vec4(vec3(LITE3D_BASE_REFLECTION_AT_ZERO_INCIDENCE), 1.0);
    surface.material.envDiffuse = LITE3D_ENV_DIFFUSE_STRENGTH;
    surface.material.envSpecular = getSpecularAmbient(uv);
    surface.material.emissionStrength = 1.0;
    surface.material.environmentUVScale = LITE3D_CUBE_MAP_UV_SCALE;
    surface.material.environmentSingleProbeIndex = -1;
    surface.material.sheen = getSheen(uv);
    surface.material.subSurfaceScattering = LITE3D_SUBSURFACE_SCATTERING;

    vec3 specular = getSpecular(uv);
    surface.material.specular = clamp(specular.x, 0.0, 1.0);
    surface.material.roughness = clamp(specular.y, LITE3D_MIN_ROUGHNESS, 1.0);
    surface.material.metallic = clamp(specular.z, 0.0, 1.0);
    surface.material.ior = getIOR(uv);
    surface.F0 = modulateF0(surface.material);
    surface.bentNormal = surface.geoNormal;

#ifdef LITE3D_ENABLE_ENVIRONMENT_TEXTURE // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_TEXTURE;
#endif

#ifdef LITE3D_ENV_PROBE_MAX // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE | LITE3D_MATERIAL_SPECULAR_PARALLAX_CORRECTION;
#endif

    return surface;
}

#include "samples:shaders/sources/ssao/resolve.glsl"

bool restoreSurface(ivec2 pixelCoord, out Surface surface)
{
    // sampling world position and world geo normal
    vec4 channel01 = texelFetch(GBuffer, ivec3(pixelCoord, 0), 0);
    // Geometry is missing
    if (isnan(channel01.w))
    {
        return false;
    }

    // sampling normal
    vec4 channel02 = texelFetch(GBuffer, ivec3(pixelCoord, 1), 0);
    // sampling albedo from fullscreen map
    vec4 channel03 = texelFetch(GBuffer, ivec3(pixelCoord, 2), 0);
    // sampling specular parameters from fullscreen map
    vec4 channel04 = texelFetch(GBuffer, ivec3(pixelCoord, 3), 0);

    vec2 albedoRG = unpack2xF12(channel03.x);
    vec2 albedoBandS = unpack2xF12(channel03.y);
    vec2 RandM = unpack2xF12(channel03.z);

    surface.materialIndex = 0u;
    surface.material.albedo = vec4(albedoRG, albedoBandS.x, 1.0);
    surface.material.emission = vec4(channel04.xyz, 1.0);
    surface.material.F0 = vec4(vec3(LITE3D_BASE_REFLECTION_AT_ZERO_INCIDENCE), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.specular = albedoBandS.y;
    surface.material.roughness = RandM.x;
    surface.material.metallic = RandM.y;
    surface.material.alpha = 1.0;
    surface.material.ior = 1.0;
    surface.material.emissionStrength = 1.0;
    surface.material.environmentUVScale = LITE3D_CUBE_MAP_UV_SCALE;
    surface.material.environmentSingleProbeIndex = -1;
    surface.material.sheen = 0.0;
    surface.material.subSurfaceScattering = LITE3D_SUBSURFACE_SCATTERING;
    surface.wv = channel01.xyz;
    surface.uv = vec2(0.0, 0.0);
    surface.normal = channel02.xyz;
    surface.geoNormal = unpackNormalFromF32(channel01.w);

    vec4 aoAndBent = resolveAO(pixelCoord, surface.geoNormal, surface.wv);
    surface.ao = channel03.w * aoAndBent.w;
    surface.bentNormal = aoAndBent.xyz;

    vec2 envDandS = unpack2xF8(channel02.w);
    surface.material.envDiffuse = envDandS.x;
    surface.material.envSpecular = envDandS.y;
    surface.F0 = modulateF0(surface.material);

#ifdef LITE3D_ENABLE_ENVIRONMENT_TEXTURE // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_TEXTURE;
#endif

#ifdef LITE3D_ENV_PROBE_MAX // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE | LITE3D_MATERIAL_SPECULAR_PARALLAX_CORRECTION;
#endif

    return true;
}

#ifdef LITE3D_FRAGMENT_SHADER
void surfaceAlphaClip(in Material material)
{
    if (material.alpha < 0.1)
        discard;
}
#endif

void surfaceAlphaClip(vec2 uv)
{
    Material material;
    material.albedo = getAlbedo(uv);
    material.alpha = material.albedo.a;
    surfaceAlphaClip(material);
}

#endif
