
// Environment probe layout
struct EnvironmentProbeStruct
{
    vec4 position;      // Probe position
    mat4 projView[6];   // View-Projection matrix for every cube face
};

#define ENV_PROBES_MAX  128