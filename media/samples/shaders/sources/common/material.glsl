#ifdef LITE3D_FRAGMENT_SHADER

uniform sampler2DArray GBuffer;

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
    surface.ao = 1.0;

    if (!isZero(wt) && !isZero(wb))
    {
        surface.normal = getNormal(uv, TBN(wn, wt, wb));
        surface.material.flags |= MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT;
    }
    else if (!isZero(wt))
    {
        surface.normal = getNormal(uv, TBN(wn, wt));
        surface.material.flags |= MATERIAL_NORMAL_MAPPING_TANGENT;
    }
    else 
    {
        surface.normal = normalize(wn);
    }

    surface.material.albedo = getAlbedo(uv);
    surface.material.alpha = surface.material.albedo.a;
    surface.material.emission = vec4(getEmission(uv), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.envSpecular = getSpecularAmbient(uv);
    surface.material.envDiffuse = 1.0;
    surface.material.emissionStrength = 1.0;

    vec3 specular = getSpecular(uv);
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.material.ior = 1.0;

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
    surface.material.f0 = vec4(vec3(0.04), 1.0);
    surface.material.normalScale = vec4(1.0, 1.0, 1.0, 1.0);
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.material.alpha = 1.0;
    surface.material.envSpecular = wv.w;
    surface.material.envDiffuse = 1.0;
    surface.material.ior = 1.0;
    surface.material.emissionStrength = 1.0;
    surface.wv = wv.xyz;
    surface.uv = uv;
    surface.normal = nw.xyz;
    surface.ao = getAmbientOcclusion(uv);

    return surface;
}

void surfaceAlphaClip(Surface surface)
{
    if (isZero(surface.material.alpha))
        discard;
}

#endif
