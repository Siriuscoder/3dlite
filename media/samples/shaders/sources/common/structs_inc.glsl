
// Environment probe layout
struct EnvironmentProbeStruct
{
    vec4 position;      // Probe position
    mat4 projView[6];   // View-Projection matrix for every cube face
    uint flags;
};

#define ENV_PROBE_FLAG_IRRADIANCE   uint(1)
#define ENV_PROBE_FLAG_SPECULAR     uint(2)

struct AngularInfo
{
    vec3 V; // view direction world space
    vec3 L; // light direction world space
    float NdotV;
    float NdotL;
    float HdotV;
    float NdotH;
};

struct LightSources
{
    vec4 position;
    vec4 direction;
    vec4 diffuse;
    float radiance;
    float influence;
    float maxInfluence;
    float minRadianceCutOff;
    float innerCone; // Rad
    float outerCone; // Rad
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    uint shadowIndex;
    uint flags;
    uint reserved;
};

