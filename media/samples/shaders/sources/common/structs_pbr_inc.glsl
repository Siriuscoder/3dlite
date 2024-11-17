#extension GL_ARB_shader_draw_parameters : require

#include "samples:shaders/sources/common/utils_inc.glsl"

#define TEXTURE_FLAG_EMPTY                           (0)
#define TEXTURE_FLAG_LOADED                          (1 << 0)
#define TEXTURE_FLAG_ALBEDO                          (1 << 1)
#define TEXTURE_FLAG_ALBEDO_ALPHA                    (1 << 2)
#define TEXTURE_FLAG_EMISSION                        (1 << 3)
#define TEXTURE_FLAG_ALPHA_MASK                      (1 << 4)
#define TEXTURE_FLAG_NORMAL                          (1 << 5)
#define TEXTURE_FLAG_SPECULAR                        (1 << 6)
#define TEXTURE_FLAG_ROUGHNESS                       (1 << 7)
#define TEXTURE_FLAG_METALLIC                        (1 << 8)
#define TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC      (1 << 9)
#define TEXTURE_FLAG_ROUGNESS_METALLIC               (1 << 10)
#define TEXTURE_FLAG_ENVIRONMENT                     (1 << 11)

struct TextureHandle
{
    sampler2D textureId;
    uint flags;
    uint32_t reserved;
};

struct TextureCubeHandle
{
    samplerCube textureId;
    uint flags;
    uint32_t reserved;
};

struct ChunkInvocationInfo
{
    mat4 model;
    mat3 normal;
    mat4 screen;
    uint materialIdx;
    uint flags;
    uint reserved01;
    uint reserved02;
};

struct PBRMaterial
{
    vec3 Albedo;
    vec3 Emission;
    vec3 F0;
    vec3 NormalScale;
    float Alpha;
    float Specular;
    float Roughness;
    float Metallic;
    float EnvDiffuse;
    float EnvSpecular;
    float Ior;
    float EmissionStrength;
    uint reserved01;
    uint reserved02;
    uint reserved03;
    uint Flags;
    TextureHandle Textures[8];
    TextureCubeHandle Environment;
};

struct PBRSurface
{
    PBRMaterial material;
    vec3 normal;
};

layout(std430) readonly buffer MultiRenderChunkInvocationBuffer 
{
    ChunkInvocationInfo chunksInvocationInfo[];
};

layout(std430) readonly buffer MultiRenderMaterialDataBuffer 
{
    PBRMaterial pbrMaterials[];
};



