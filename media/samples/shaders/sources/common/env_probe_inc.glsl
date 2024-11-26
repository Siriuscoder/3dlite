
// Environment probe layout
struct EnvironmentProbeStruct
{
    vec4 position;      // Probe position
    mat4 projView[6];   // View-Projection matrix for every cube face
    uint flags;
};

#define ENV_PROBE_FLAG_IRRADIANCE   uint(1)
#define ENV_PROBE_FLAG_SPECULAR     uint(2)
