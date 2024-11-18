#extension GL_ARB_shader_draw_parameters : require

layout(std430) readonly buffer MultiRenderChunkInvocationBuffer 
{
    ChunkInvocationInfo chunksInvocationInfo[];
};

layout(std430) readonly buffer MultiRenderMaterialDataBuffer 
{
    Material materials[];
};

uniform sampler2DArray GBuffer;

float getAmbientOcclusion(vec2 uv);

ChunkInvocationInfo getInvocationInfo()
{
    return chunksInvocationInfo[gl_DrawID];
}

Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb)
{
    Surface surface;
    surface.transform = getInvocationInfo();
    surface.material = materials[surface.transform.materialIdx];
    surface.uv = uv;
    surface.wv = wv;

    for (int = 0; i < 8; ++i)
    {
        if (surface.material.textures[i].flags & TEXTURE_FLAG_LOADED)
        {
            if (surface.material.textures[i].flags & TEXTURE_FLAG_ALBEDO)
            {
                surface.material.albedo *= texture(surface.material.textures[i].textureId, uv).rgb;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_ALBEDO_ALPHA)
            {
                vec4 albedo = texture(surface.material.textures[i].textureId, uv).rgba;
                surface.material.albedo *= albedo.rgb;
                surface.material.alpha *= albedo.a;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_EMISSION)
            {
                surface.material.emission = texture(surface.material.textures[i].textureId, uv).rgb;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_ALPHA_MASK)
            {
                surface.material.alpha *= texture(surface.material.textures[i].textureId, uv).r;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_NORMAL)
            {
                if (surface.material.flags & MATERIAL_NORMAL_MAPPING_TANGENT)
                {
                    vec2 nl = texture(surface.material.textures[i].textureId, uv).rg;
                    surface.normal = calcNormal(nl, TBN(wn, wt), surface.material.NormalScale);
                }
                else if (surface.material.flags & MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT)
                {
                    vec2 nl = texture(surface.material.textures[i].textureId, uv).rg;
                    surface.normal = calcNormal(nl, TBN(wn, wt, wb), surface.material.NormalScale);
                }
                else
                {
                    surface.normal = normalize(wn);
                }
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_SPECULAR)
            {
                surface.material.Specular *= texture(surface.material.textures[i].textureId, uv).r;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_ROUGHNESS)
            {
                surface.material.Roughness *= texture(surface.material.textures[i].textureId, uv).r;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_METALLIC)
            {
                surface.material.Metallic *= texture(surface.material.textures[i].textureId, uv).r;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC)
            {
                vec3 srm = texture(surface.material.textures[i].textureId, uv).rgb;
                surface.material.Specular *= sr.r;
                surface.material.Roughness *= sr.g;
                surface.material.Metallic *= sr.b;
            }
            else if (surface.material.textures[i].flags & TEXTURE_FLAG_ROUGNESS_METALLIC)
            {
                vec2 rm = texture(surface.material.textures[i].textureId, uv).gb;
                surface.material.Roughness *= rm.r;
                surface.material.Metallic *= rm.g;
            }
        }
        else
        {
            break;
        }
    }
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
    // Material index was stored in vw.w
    surface.material = materials[int(round(vw.w))];
    surface.material.albedo = albedo.rgb;
    surface.material.emission = emission;
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.wv = wv.xyz;
    surface.uv = uv;
    surface.normal = nw.xyz;
    surface.ao = getAmbientOcclusion(uv);
}

void surfaceAlphaClip(Surface surface)
{
    if (isZero(surface.material.alpha))
        discard;
}
