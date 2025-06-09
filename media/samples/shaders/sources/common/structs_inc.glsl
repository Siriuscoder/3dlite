#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_bindless_texture : require
#endif

#ifdef LITE3D_ENV_PROBE_MAX
#extension GL_ARB_texture_cube_map_array : require
#endif

// Environment probe layout
struct EnvironmentProbeStruct
{
    vec4 position;      // Probe position
    mat4 projView[6];   // View-Projection matrix for every cube face
    uint flags;
};

#define LITE3D_ENV_PROBE_FLAG_IRRADIANCE   uint(1)
#define LITE3D_ENV_PROBE_FLAG_SPECULAR     uint(2)

struct AngularInfo
{
    vec3 viewDir;   // view direction
    vec3 lightDir;  // light direction
    float lightDistance;
    bool isOutside;
    float NdotV;
    float NdotL;
    float HdotV;
    float NdotH;
    float LdotV;
};

struct LightSource
{
    vec4 position;
    vec4 direction;
    vec4 diffuse;
    float radiance;
    float influenceDistance;
    float maxInfluence;
    float minRadianceCutOff;
    float innerCone; // Rad
    float outerCone; // Rad
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    float lightSize;
    uint shadowIndex;
    uint flags;
};

#define LITE3D_LIGHT_POINT                      uint(1 << 0)
#define LITE3D_LIGHT_DIRECTIONAL                uint(1 << 1)
#define LITE3D_LIGHT_SPOT                       uint(1 << 2)
#define LITE3D_LIGHT_ENABLED                    uint(1 << 3)
#define LITE3D_LIGHT_CASTSHADOW                 uint(1 << 4)
#define LITE3D_LIGHT_CASTSHADOW_PCF3x3          uint(1 << 5)
#define LITE3D_LIGHT_CASTSHADOW_PCF_ADAPTIVE    uint(1 << 6)
#define LITE3D_LIGHT_CASTSHADOW_POISSON         uint(1 << 7)
#define LITE3D_LIGHT_CASTSHADOW_SSS             uint(1 << 8)

#ifndef LITE3D_SSS_MAX_ADAPTIVE_DEPTH_THRESHOLD
#define LITE3D_SSS_MAX_ADAPTIVE_DEPTH_THRESHOLD             0.01    // Min Depth clipping to avoid accuracy artifacts
#endif

#ifndef LITE3D_SSS_MAX_STEPS
#define LITE3D_SSS_MAX_STEPS                                16      // Max ray steps, affects quality and performance.
#endif

#ifndef LITE3D_SSS_MAX_RAY_DISTANCE
#define LITE3D_SSS_MAX_RAY_DISTANCE                         0.75    // Max shadow length, longer shadows are less accurate.
#endif

#ifndef LITE3D_SSS_MAX_DEPTH_THRESHOLD
#define LITE3D_SSS_MAX_DEPTH_THRESHOLD                      0.5     // Depth testing thickness.
#endif

#define LITE3D_SSS_STEP_LENGTH                              (LITE3D_SSS_MAX_RAY_DISTANCE / float(LITE3D_SSS_MAX_STEPS))

#ifndef LITE3D_SHADOW_MAX_ADAPTIVE_BIAS
#define LITE3D_SHADOW_MAX_ADAPTIVE_BIAS                     0.0028
#endif

#ifndef LITE3D_SHADOW_MIN_ADAPTIVE_BIAS
#define LITE3D_SHADOW_MIN_ADAPTIVE_BIAS                     0.0008
#endif

#ifndef LITE3D_SHADOW_MIN_ADAPTIVE_FILTER_SIZE 
#define LITE3D_SHADOW_MIN_ADAPTIVE_FILTER_SIZE              1.0
#endif

#ifndef LITE3D_SHADOW_MAX_ADAPTIVE_FILTER_SIZE
#define LITE3D_SHADOW_MAX_ADAPTIVE_FILTER_SIZE              1.5
#endif

#ifndef LITE3D_SHADOW_MIN_ADAPTIVE_STEP
#define LITE3D_SHADOW_MIN_ADAPTIVE_STEP                     0.5
#endif

#define LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT              uint(1 << 0)
#define LITE3D_MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT    uint(1 << 1)
#define LITE3D_MATERIAL_ENVIRONMENT_TEXTURE                 uint(1 << 2)
#define LITE3D_MATERIAL_ENVIRONMENT_MULTI_PROBE             uint(1 << 3)
#define LITE3D_MATERIAL_ENVIRONMENT_SINGLE_PROBE            uint(1 << 4)

#define TEXTURE_FLAG_EMPTY                                  uint(0)
#define TEXTURE_FLAG_LOADED                                 uint(1 << 0)
#define TEXTURE_FLAG_ALBEDO                                 uint(1 << 1)
#define TEXTURE_FLAG_EMISSION                               uint(1 << 2)
#define TEXTURE_FLAG_ALPHA_MASK                             uint(1 << 3)
#define TEXTURE_FLAG_NORMAL_RG                              uint(1 << 4)
#define TEXTURE_FLAG_NORMAL_RGB                             uint(1 << 5)
#define TEXTURE_FLAG_AO                                     uint(1 << 6)
#define TEXTURE_FLAG_SPECULAR                               uint(1 << 7)
#define TEXTURE_FLAG_ROUGHNESS                              uint(1 << 8)
#define TEXTURE_FLAG_METALLIC                               uint(1 << 9)
#define TEXTURE_FLAG_SPECULAR_ROUGNESS_METALLIC             uint(1 << 10)
#define TEXTURE_FLAG_ROUGNESS_METALLIC                      uint(1 << 11)
#define TEXTURE_FLAG_ENVIRONMENT                            uint(1 << 12)
#define TEXTURE_FLAG_ENVIRONMENT_PROBE                      uint(1 << 13)

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE

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

struct TextureCubeArrayHandle
{
    samplerCubeArray textureId;
    uint flags;
    uint reserved;
};

struct ChunkInvocationInfo
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    uint materialIdx;
    uint flags;
    uint skeletonBufferIndex;
    uint reserved02;
};

ChunkInvocationInfo getInvocationInfo();

#endif

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
    float environmentUVScale;
    uint environmentSingleProbeIndex;
    float sheen;
    uint flags;
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    TextureHandle slot[8];
    TextureCubeHandle environment;
    TextureCubeArrayHandle environmentProbe;
#endif
};

struct Surface
{
    Material material;
    uint index;
    vec3 wv;
    vec2 uv;
    vec3 normal;
    float ao;
};
