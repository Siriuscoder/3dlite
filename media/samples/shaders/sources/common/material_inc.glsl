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

#define MATERIAL_NORMAL_MAPPING_TANGENT              (1 << 0)
#define MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT    (1 << 1)

struct TextureHandle
{
    sampler2D textureId;
    uint flags;
    uint reserved;
};

struct TextureCubeHandle
{
    samplerCube textureId;
    uint flags;
    uint reserved;
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

struct Material
{
    vec3 albedo;
    vec3 emission;
    vec3 f0;
    vec3 normalScale;
    float alpha;
    float specular;
    float roughness;
    float metallic;
    float envDiffuse;
    float envSpecular;
    float ior;
    float emissionStrength;
    uint reserved01;
    uint reserved02;
    uint reserved03;
    uint flags;
    TextureHandle textures[8];
    TextureCubeHandle environment;
};

struct Surface
{
    ChunkInvocationInfo transform;
    Material material;
    vec3 wv;
    vec2 uv;
    vec3 normal;
    float ao;
};

ChunkInvocationInfo getInvocationInfo();
Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb);
Surface restoreSurface(vec2 uv);
void surfaceAlphaClip(Surface surface);

