#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/common/utils_pbr_inc.glsl"

layout(location = 0) out vec4 channel01;
layout(location = 1) out vec4 channel02;
layout(location = 2) out vec4 channel03;
layout(location = 3) out vec4 channel04;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

vec3 getNormal(vec3 nt, mat3 tbn, vec3 normalScale)
{
    // put normal in [-1,1] range in tangent space
    nt = 2.0 * clamp(nt, 0.0, 1.0) - 1.0;
    // Refix Z (may be missing)
    nt.z = sqrt(1.0 - dot(nt.xy, nt.xy));
    // trasform normal to world space using common TBN
    return normalize(tbn * normalize(nt * normalScale));
}

void prepareMaterial(vec2 uv, inout PBRSurface surface)
{
    for (int = 0; i < 8; ++i)
    {
        if (surface.material.Textures[i].flags & TEXTURE_FLAG_LOADED)
        {
            if (surface.material.Textures[i].flags & TEXTURE_FLAG_ALBEDO)
            {
                surface.material.Albedo *= texture(surface.material.Textures[i].textureId, uv).rgb;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_ALBEDO_ALPHA)
            {
                vec4 albedo = texture(surface.material.Textures[i].textureId, uv).rgba;
                surface.material.Albedo *= albedo.rgb;
                surface.material.Alpha *= albedo.a;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_EMISSION)
            {
                surface.material.Emission = texture(surface.material.Textures[i].textureId, uv).rgb;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_ALPHA_MASK)
            {
                surface.material.Alpha *= texture(surface.material.Textures[i].textureId, uv).r;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_NORMAL)
            {
                // Get world-space normal
#if defined(NORMAL_MAPPING_OFF)
                surface.normal = normalize(iwn);
#elif defined(NORMAL_MAPPING_TANGENT)
                surface.normal = getNormal(iuv, TBN(iwn, iwt), surface.material.NormalScale);
#else
                surface.normal = getNormal(iuv, TBN(iwn, iwt, iwb), surface.material.NormalScale);
#endif
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_SPECULAR)
            {
                surface.material.Specular *= texture(surface.material.Textures[i].textureId, uv).r;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_ROUGHNESS)
            {
                surface.material.Roughness *= texture(surface.material.Textures[i].textureId, uv).r;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_METALLIC)
            {
                surface.material.Metallic *= texture(surface.material.Textures[i].textureId, uv).r;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC)
            {
                vec3 srm = texture(surface.material.Textures[i].textureId, uv).rgb;
                surface.material.Specular *= sr.r;
                surface.material.Roughness *= sr.g;
                surface.material.Metallic *= sr.b;
            }
            else if (surface.material.Textures[i].flags & TEXTURE_FLAG_ROUGNESS_METALLIC)
            {
                vec2 rm = texture(surface.material.Textures[i].textureId, uv).gb;
                surface.material.Roughness *= rm.r;
                surface.material.Metallic *= rm.g;
            }
        }
    }
}

void main()
{
    PBRSurface surface;
    ChunkInvocationInfo chunkInfo = chunksInvocationInfo[gl_DrawID];
    surface.material = pbrMaterials[chunkInfo.materialIdx];

    prepareMaterial(iuv, surface);

    channel01 = vec4(iwv, float(chunkInfo.materialIdx));
    channel02 = vec4(surface.normal, surface.material.Emission.r);
    channel03 = vec4(surface.material.Albedo, surface.material.Emission.g);
    channel04 = vec4(surface.material.Specular, surface.material.Roughness, surface.material.Metallic, surface.material.Emission.b);
}