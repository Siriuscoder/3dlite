#if defined(LITE3D_VERTEX_SHADER) || defined(LITE3D_FRAGMENT_SHADER)
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

#ifdef LITE3D_VERTEX_SKELETON_DEFORM

layout(std430) readonly buffer SkeletonTransformBuffer 
{
    mat4 skeletonTransform[];
};

void skeletonDeform(inout ChunkInvocationInfo chunkInfo, ivec4 boneIndexes, vec4 boneWeights)
{
    if (chunkInfo.skeletonTransformIndex < 0)
    {
        return;
    }

    mat4 deform = mat4(0.0);
    for (int i = 0; i < 4; i++)
    {
        if (boneIndexes[i] >= 0)
        {
            deform += boneWeights[i] * skeletonTransform[chunkInfo.skeletonTransformIndex + boneIndexes[i]];
        }
    }

    chunkInfo.modelMatrix *= deform;
    // TBD: consider to take correct normalMatrix
    //chunkInfo.normalMatrix = transpose(inverse(mat3(chunkInfo.modelMatrix)));
}

#endif

ChunkInvocationInfo getInvocationInfo()
{
    drawID = gl_DrawIDARB + gl_InstanceID;
    int chunkIndex = chunksIndexInvocationInfo[drawID/4][drawID % 4];
    return chunksInvocationInfo[chunkIndex];
}

#elif defined(LITE3D_FRAGMENT_SHADER)

layout(location = 1) flat in int drawID;

ChunkInvocationInfo getInvocationInfo()
{
    int chunkIndex = chunksIndexInvocationInfo[drawID/4][drawID % 4];
    return chunksInvocationInfo[chunkIndex];
}

#include "samples:shaders/sources/pbr/roughness_filter.glsl"

Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb)
{
    Surface surface;
    uint materialIndex = getInvocationInfo().materialIndex;
    surface.material = materials[materialIndex];
    surface.materialIndex = materialIndex;
    surface.uv = uv;
    surface.wv = wv;
    surface.normal = surface.geoNormal = surface.bentNormal = normalize(wn);
    surface.ao = 1.0;

    for (int i = 0; i < 8; ++i)
    {
        if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_LOADED))
        {
            if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALBEDO))
            {
                vec4 albedo = texture(sampler2D(surface.material.slot[i].textureId), uv);
                surface.material.albedo *= vec4(albedo.rgb, 1.0);
                surface.material.alpha *= albedo.a;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_EMISSION))
            {
                surface.material.emission *= vec4(texture(sampler2D(surface.material.slot[i].textureId), uv).rgb, 1.0);
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALPHA_MASK))
            {
                surface.material.alpha *= texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_NORMAL_RG))
            {
                if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT))
                {
                    if (!isZero(wt))
                    {
                        vec2 nl = texture(sampler2D(surface.material.slot[i].textureId), uv).rg;
                        surface.normal = calcNormal(nl, TBN(wn, wt), surface.material.normalScale.xyz);
                    }
                }
                else if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT))
                {
                    if (!isZero(wt) && !isZero(wb))
                    {
                        vec2 nl = texture(sampler2D(surface.material.slot[i].textureId), uv).rg;
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
                        vec3 nl = texture(sampler2D(surface.material.slot[i].textureId), uv).rgb;
                        surface.normal = calcNormal(nl, TBN(wn, wt), surface.material.normalScale.xyz);
                    }
                }
                else if (hasFlag(surface.material.flags, LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT))
                {
                    if (!isZero(wt) && !isZero(wb))
                    {
                        vec3 nl = texture(sampler2D(surface.material.slot[i].textureId), uv).rgb;
                        surface.normal = calcNormal(nl, TBN(wn, wt, wb), surface.material.normalScale.xyz);
                    }
                }
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_AO))
            {
                surface.ao = texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_SPECULAR))
            {
                surface.material.specular *= texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ROUGHNESS))
            {
                surface.material.roughness *= texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_METALLIC))
            {
                surface.material.metallic *= texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC))
            {
                vec3 srm = texture(sampler2D(surface.material.slot[i].textureId), uv).rgb;
                surface.material.specular *= srm.r;
                surface.material.roughness *= srm.g;
                surface.material.metallic *= srm.b;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ROUGNESS_METALLIC))
            {
                vec2 rm = texture(sampler2D(surface.material.slot[i].textureId), uv).gb;
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
    surface.material.roughness = filterRoughness(surface.normal, surface.material.roughness);
    surface.material.metallic = clamp(surface.material.metallic, 0.0, 1.0);
    surface.material.emission *= surface.material.emissionStrength;
    surface.F0 = modulateF0(surface.material);
    
    return surface;
}

void surfaceAlphaClip(vec2 uv)
{
    Surface surface;
    uint materialIndex = getInvocationInfo().materialIndex;
    surface.material = materials[materialIndex];

    for (int i = 0; i < 8; ++i)
    {
        if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_LOADED))
        {
            if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALBEDO))
            {
                vec4 albedo = texture(sampler2D(surface.material.slot[i].textureId), uv);
                surface.material.albedo *= vec4(albedo.rgb, 1.0);
                surface.material.alpha *= albedo.a;
            }
            else if (hasFlag(surface.material.slot[i].flags, TEXTURE_FLAG_ALPHA_MASK))
            {
                surface.material.alpha *= texture(sampler2D(surface.material.slot[i].textureId), uv).r;
            }
        }
        else
        {
            break;
        }
    }

    surfaceAlphaClip(surface.material);
}

void surfaceAlphaClip(in Material material)
{
    if (material.alpha < 0.1)
        discard;
}

#elif defined(LITE3D_COMPUTE_SHADER)

uniform sampler2DArray GBuffer;

#include "samples:shaders/sources/ssao/resolve.glsl"

bool restoreSurface(ivec2 pixelCoord, out Surface surface)
{
    // sampling fragment world position and world geo normal
    vec4 channel01 = texelFetch(GBuffer, ivec3(pixelCoord, 0), 0);
    // Geometry is missing
    if (isnan(channel01.w))
    {
        return false;
    }

    // sampling shading normal in world space
    vec4 channel02 = texelFetch(GBuffer, ivec3(pixelCoord, 1), 0);
    // sampling albedo and marterial params
    vec4 channel03 = texelFetch(GBuffer, ivec3(pixelCoord, 2), 0);
    // sampling emission
    vec4 channel04 = texelFetch(GBuffer, ivec3(pixelCoord, 3), 0);

    vec2 albedoRG = unpack2xF12(channel03.x);
    vec2 albedoBandS = unpack2xF12(channel03.y);
    vec2 RandM = unpack2xF12(channel03.z);

    // unpack material index from channel02.w
    surface.materialIndex = uint(channel02.w);

    surface.material = materials[surface.materialIndex];
    surface.material.albedo = vec4(albedoRG, albedoBandS.x, 1.0);
    surface.material.emission = vec4(channel04.xyz, 1.0);
    surface.material.specular = albedoBandS.y;
    surface.material.roughness = RandM.x;
    surface.material.metallic = RandM.y;
    surface.wv = channel01.xyz;
    surface.uv = vec2(0.0, 0.0);
    surface.normal = channel02.xyz;
    surface.geoNormal = unpackNormalFromF32(channel01.w);

    vec4 aoAndBent = resolveAO(pixelCoord, surface.geoNormal, surface.wv);
    surface.ao = channel03.w * aoAndBent.w;
    surface.bentNormal = aoAndBent.xyz;
    surface.F0 = modulateF0(surface.material);
    
    return true;
}

#endif
