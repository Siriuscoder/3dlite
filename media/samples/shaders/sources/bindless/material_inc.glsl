#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_bindless_texture : require

#define TEXTURE_FLAG_EMPTY                           uint(0)
#define TEXTURE_FLAG_LOADED                          uint(1 << 0)
#define TEXTURE_FLAG_ALBEDO                          uint(1 << 1)
#define TEXTURE_FLAG_EMISSION                        uint(1 << 2)
#define TEXTURE_FLAG_ALPHA_MASK                      uint(1 << 3)
#define TEXTURE_FLAG_NORMAL                          uint(1 << 4)
#define TEXTURE_FLAG_AO                              uint(1 << 5)
#define TEXTURE_FLAG_SPECULAR                        uint(1 << 6)
#define TEXTURE_FLAG_ROUGHNESS                       uint(1 << 7)
#define TEXTURE_FLAG_METALLIC                        uint(1 << 8)
#define TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC      uint(1 << 9)
#define TEXTURE_FLAG_ROUGNESS_METALLIC               uint(1 << 10)
#define TEXTURE_FLAG_ENVIRONMENT                     uint(1 << 11)

#define MATERIAL_NORMAL_MAPPING_TANGENT              uint(1 << 0)
#define MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT    uint(1 << 1)

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
    mat4 screen;
    mat4 normal;
    uint materialIdx;
    uint flags;
    uint reserved01;
    uint reserved02;
};

struct Material
{
    vec4 albedo;
    vec4 emission;
    vec4 f0;
    vec4 normalScale;
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
    TextureHandle slot[8];
    TextureCubeHandle environment;
};

struct Surface
{
    ChunkInvocationInfo transform;
    Material material;
    uint index;
    vec3 wv;
    vec2 uv;
    vec3 normal;
    float ao;
};

ChunkInvocationInfo getInvocationInfo();
Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb);
Surface restoreSurface(vec2 uv);
void surfaceAlphaClip(Surface surface);
void surfaceAlphaClip(vec2 uv);
