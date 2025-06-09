#if defined(LITE3D_VERTEX_SHADER) || !defined(LITE3D_DISABLE_INVOCATION_METHOD)
layout(std430) readonly buffer MultiRenderChunkInvocationBuffer 
{
    ChunkInvocationInfo chunksInvocationInfo[];
};

layout(std140) uniform MultiRenderChunkInvocationIndexBuffer
{
    ivec4 chunksIndexInvocationInfo[4000];
};
#endif

layout(std430) readonly buffer MultiRenderMaterialDataBuffer 
{
    Material materials[];
};

#ifdef LITE3D_VERTEX_SHADER

layout(location = 1) flat out int drawID;

ChunkInvocationInfo getInvocationInfo()
{
    drawID = gl_DrawIDARB + gl_InstanceID;
    int chunkIndex = chunksIndexInvocationInfo[drawID/4][int(mod(drawID, 4))];
    return chunksInvocationInfo[chunkIndex];
}

#elif defined(LITE3D_FRAGMENT_SHADER)

#ifndef LITE3D_DISABLE_INVOCATION_METHOD

layout(location = 1) flat in int drawID;

ChunkInvocationInfo getInvocationInfo()
{
    int chunkIndex = chunksIndexInvocationInfo[drawID/4][int(mod(drawID, 4))];
    return chunksInvocationInfo[chunkIndex];
}

Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb)
{
    Surface surface;
    surface.transform = getInvocationInfo();
    surface.material = materials[surface.transform.materialIdx];
    surface.index = surface.transform.materialIdx;
    surface.uv = uv;
    surface.wv = wv;
    surface.normal = normalize(wn);
    surface.ao = 1.0;

    for (int i = 0; i < 8; ++i)
    {
        if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_LOADED))
        {
            if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALBEDO))
            {
                vec4 albedo = texture(surface.material.slot[i].textureId, uv);
                surface.material.albedo *= vec4(albedo.rgb, 1.0);
                surface.material.alpha *= albedo.a;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_EMISSION))
            {
                surface.material.emission *= vec4(texture(surface.material.slot[i].textureId, uv).rgb, 1.0);
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALPHA_MASK))
            {
                surface.material.alpha *= texture(surface.material.slot[i].textureId, uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_NORMAL_RG))
            {
                if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT))
                {
                    if (!isZero(wt))
                    {
                        vec2 nl = texture(surface.material.slot[i].textureId, uv).rg;
                        surface.normal = calcNormal(nl, TBN(wn, wt), surface.material.normalScale.xyz);
                    }
                }
                else if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT))
                {
                    if (!isZero(wt) && !isZero(wb))
                    {
                        vec2 nl = texture(surface.material.slot[i].textureId, uv).rg;
                        surface.normal = calcNormal(nl, TBN(wn, wt, wb), surface.material.normalScale.xyz);
                    }
                }
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_NORMAL_RGB))
            {
                if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT))
                {
                    if (!isZero(wt))
                    {
                        vec3 nl = texture(surface.material.slot[i].textureId, uv).rgb;
                        surface.normal = calcNormal(nl, TBN(wn, wt), surface.material.normalScale.xyz);
                    }
                }
                else if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT))
                {
                    if (!isZero(wt) && !isZero(wb))
                    {
                        vec3 nl = texture(surface.material.slot[i].textureId, uv).rgb;
                        surface.normal = calcNormal(nl, TBN(wn, wt, wb), surface.material.normalScale.xyz);
                    }
                }
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_AO))
            {
                surface.ao = texture(surface.material.slot[i].textureId, uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_SPECULAR))
            {
                surface.material.specular *= texture(surface.material.slot[i].textureId, uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ROUGHNESS))
            {
                surface.material.roughness *= texture(surface.material.slot[i].textureId, uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_METALLIC))
            {
                surface.material.metallic *= texture(surface.material.slot[i].textureId, uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC))
            {
                vec3 srm = texture(surface.material.slot[i].textureId, uv).rgb;
                surface.material.specular *= srm.r;
                surface.material.roughness *= srm.g;
                surface.material.metallic *= srm.b;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ROUGNESS_METALLIC))
            {
                vec2 rm = texture(surface.material.slot[i].textureId, uv).gb;
                surface.material.roughness *= rm.r;
                surface.material.metallic *= rm.g;
            }
        }
        else
        {
            break;
        }
    }
    
    surface.material.specular = clamp(surface.material.specular, 0.0, 1.0);
    surface.material.roughness = clamp(surface.material.roughness, LITE3D_MIN_ROUGHNESS, 1.0);
    surface.material.metallic = clamp(surface.material.metallic, 0.0, 1.0);
    surface.material.emission *= surface.material.emissionStrength;
    return surface;
}

void surfaceAlphaClip(vec2 uv)
{
    Surface surface;
    surface.transform = getInvocationInfo();
    surface.material = materials[surface.transform.materialIdx];

    for (int i = 0; i < 8; ++i)
    {
        if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_LOADED))
        {
            if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALBEDO))
            {
                vec4 albedo = texture(surface.material.slot[i].textureId, uv);
                surface.material.albedo *= vec4(albedo.rgb, 1.0);
                surface.material.alpha *= albedo.a;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALPHA_MASK))
            {
                surface.material.alpha *= texture(surface.material.slot[i].textureId, uv).r;
            }
        }
        else
        {
            break;
        }
    }

    surfaceAlphaClip(surface.material);
}

#endif

float getAmbientOcclusion(vec2 uv);
uniform sampler2DArray GBuffer;

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
    surface.index = uint(round(wv.w)); // Material index was stored in wv.w
    surface.material = materials[surface.index];
    surface.material.albedo = vec4(albedo.rgb, 1.0);
    surface.material.emission = vec4(emission, 1.0);
    surface.material.specular = specular.x;
    surface.material.roughness = specular.y;
    surface.material.metallic = specular.z;
    surface.wv = wv.xyz;
    surface.uv = uv;
    surface.normal = nw.xyz;
    surface.ao = getAmbientOcclusion(uv);

    return surface;
}

void surfaceAlphaClip(in Material material)
{
    if (isZero(material.alpha))
        discard;
}

#endif
