
// Environment probe layout
struct EnvironmentProbeStruct
{
    vec4 position;      // Probe position
    mat4 projView[6];   // View-Projection matrix for every cube face
    int flags;
};

#define ENV_PROBE_FLAG_IRRADIANCE   1
#define ENV_PROBE_FLAG_SPECULAR     2
#define ENV_PROBES_MAX              128