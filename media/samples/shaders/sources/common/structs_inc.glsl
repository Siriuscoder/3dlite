
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
