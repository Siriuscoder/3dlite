#ifdef LITE3D_FRAGMENT_SHADER

uniform sampler2DArray GBuffer;

#ifndef LITE3D_CUBE_MAP_UV_SCALE
#define LITE3D_CUBE_MAP_UV_SCALE 1.0
#endif

#ifndef LITE3D_ENV_DIFFUSE_STRENGTH
#define LITE3D_ENV_DIFFUSE_STRENGTH 1.0
#endif

vec4 getAlbedo(vec2 uv);
vec3 getEmission(vec2 uv);
vec3 getNormal(vec2 uv, mat3 tbn);
vec3 getSpecular(vec2 uv);
float getAmbientOcclusion(vec2 uv);
float getSpecularAmbient(vec2 uv);

Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb)
{
    Surface surface;
    surface.index = 0u;
    surface.uv = uv;
    surface.wv = wv;
    surface.ao = getAmbientOcclusion(uv);

    if (!isZero(wt) && !isZero(wb))
    {
        surface.normal = getNormal(uv, TBN(wn, wt, wb));
        surface.material.flags |= LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT;
    }
    else if (!isZero(wt))
    {
        surface.normal = getNormal(uv, TBN(wn, wt));
        surface.material.flags |= LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT;
    }
    else 
    {
        surface.normal = normalize(wn);
    }

    surface.material.albedo = getAlbedo(uv);
    surface.material.alpha = surface.material.albedo.a;
    surface.material.emission = vec4(getEmission(uv), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.f0 = vec4(vec3(LITE3D_BASE_REFLECTION_AT_ZERO_INCIDENCE), 1.0);
    surface.material.envDiffuse = LITE3D_ENV_DIFFUSE_STRENGTH;
    surface.material.envSpecular = getSpecularAmbient(uv);
    surface.material.emissionStrength = 1.0;
    surface.material.environmentUVScale = LITE3D_CUBE_MAP_UV_SCALE;
    surface.material.environmentSingleProbeIndex = 0u;

    vec3 specular = getSpecular(uv);
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.material.ior = 1.0;

#ifdef LITE3D_ENABLE_ENVIRONMENT_TEXTURE // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_TEXTURE;
#endif

#ifdef LITE3D_ENV_PROBE_MAX // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE;
#endif

    return surface;
}

Surface restoreSurface(vec2 uv)
{
    // sampling normal in world space from fullscreen normal map
    vec4 nw = texture(GBuffer, vec3(uv, 1));
    // Non shaded fragment
    if (isZero(nw.xyz))
        discard;

    // sampling fragment position in world space from fullscreen normal map
    vec4 wv = texture(GBuffer, vec3(uv, 0));
    // sampling albedo from fullscreen map
    vec4 albedo = texture(GBuffer, vec3(uv, 2));
    // sampling specular parameters from fullscreen map
    vec4 specular = texture(GBuffer, vec3(uv, 3));
    // Emission
    vec3 emission = vec3(nw.a, albedo.a, specular.a);

    Surface surface;
    surface.index = 0u;
    surface.material.albedo = vec4(albedo.rgb, 1.0);
    surface.material.emission = vec4(emission, 1.0);
    surface.material.f0 = vec4(vec3(LITE3D_BASE_REFLECTION_AT_ZERO_INCIDENCE), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.material.alpha = 1.0;
    surface.material.envSpecular = wv.w;
    surface.material.envDiffuse = LITE3D_ENV_DIFFUSE_STRENGTH;
    surface.material.ior = 1.0;
    surface.material.emissionStrength = 1.0;
    surface.material.environmentUVScale = LITE3D_CUBE_MAP_UV_SCALE;
    surface.material.environmentSingleProbeIndex = 0u;
    surface.wv = wv.xyz;
    surface.uv = uv;
    surface.normal = nw.xyz;
    surface.ao = getAmbientOcclusion(uv);

#ifdef LITE3D_ENABLE_ENVIRONMENT_TEXTURE // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_TEXTURE;
#endif

#ifdef LITE3D_ENV_PROBE_MAX // Setup by the engine 
    surface.material.flags |= LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE;
#endif

    return surface;
}

void surfaceAlphaClip(in Material material)
{
    if (isZero(material.alpha))
        discard;
}

void surfaceAlphaClip(vec2 uv)
{
    Material material;
    material.albedo = getAlbedo(uv);
    material.alpha = material.albedo.a;
    surfaceAlphaClip(material);
}

#endif
