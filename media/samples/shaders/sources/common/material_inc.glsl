#define MATERIAL_NORMAL_MAPPING_TANGENT              uint(1 << 0)
#define MATERIAL_NORMAL_MAPPING_TANGENT_BITANGENT    uint(1 << 1)

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
    uint flags;
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
