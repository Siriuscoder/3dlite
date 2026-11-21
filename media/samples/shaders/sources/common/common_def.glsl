#ifndef LITE3D_SSS_MIN_DEPTH_THRESHOLD
#define LITE3D_SSS_MIN_DEPTH_THRESHOLD                      0.01    // Min Depth clipping to avoid accuracy artifacts
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

#ifndef LITE3D_SSS_FADE_START
#define LITE3D_SSS_FADE_START                               6.5
#endif

#ifndef LITE3D_SSS_FADE_END
#define LITE3D_SSS_FADE_END                                 10.0
#endif

#define LITE3D_SSS_STEP_LENGTH                              (LITE3D_SSS_MAX_RAY_DISTANCE / float(LITE3D_SSS_MAX_STEPS))

#ifndef LITE3D_SHADOW_MAX_BIAS
#define LITE3D_SHADOW_MAX_BIAS                              0.0028
#endif

#ifndef LITE3D_SHADOW_MIN_BIAS
#define LITE3D_SHADOW_MIN_BIAS                              0.0008
#endif

#ifndef LITE3D_SHADOW_PCF_MIN_FILTER_SIZE 
#define LITE3D_SHADOW_PCF_MIN_FILTER_SIZE                   1.0
#endif

#ifndef LITE3D_SHADOW_PCF_MAX_FILTER_SIZE
#define LITE3D_SHADOW_PCF_MAX_FILTER_SIZE                   1.5
#endif

#ifndef LITE3D_SHADOW_PCF_MIN_STEP
#define LITE3D_SHADOW_PCF_MIN_STEP                          0.5
#endif

#ifndef LITE3D_SHADOW_VSM_BIAS
#define LITE3D_SHADOW_VSM_BIAS                              0.0001
#endif

#ifndef LITE3D_SHADOW_VSM_LIGHT_BLEEDING_REDUCTION
#define LITE3D_SHADOW_VSM_LIGHT_BLEEDING_REDUCTION          0.2
#endif

#ifndef LITE3D_VSM_BLUR_MAX_RADIUS
#define LITE3D_VSM_BLUR_MAX_RADIUS                          24
#endif

#ifndef LITE3D_SHADOW_CSM_CASCADE_COUNT
#define LITE3D_SHADOW_CSM_CASCADE_COUNT                     1
#endif

#ifndef LITE3D_REFRACTION_BLUR_SCALE                        
#define LITE3D_REFRACTION_BLUR_SCALE                        0.015
#endif

#ifndef LITE3D_REFRACTION_THICKNESS                        
#define LITE3D_REFRACTION_THICKNESS                         10.0
#endif

#ifndef LITE3D_ENV_PROBE_MIN_WEIGHT
#define LITE3D_ENV_PROBE_MIN_WEIGHT                         0.0001
#endif

#ifndef LITE3D_ENV_PROBE_MIN_SPACING
#define LITE3D_ENV_PROBE_MIN_SPACING                        2.0
#endif

#ifndef LITE3D_ENV_PROBE_VISIBILITY_BIAS
#define LITE3D_ENV_PROBE_VISIBILITY_BIAS                    1.0
#endif

#ifndef LITE3D_ENV_PROBE_VISIBILITY_MAX_SOFTNESS
#define LITE3D_ENV_PROBE_VISIBILITY_MAX_SOFTNESS            0.5
#endif

#ifndef LITE3D_ENV_PROBE_PARALAX_CORRECT_COARSE_STEPS
#define LITE3D_ENV_PROBE_PARALAX_CORRECT_COARSE_STEPS       8
#endif

#ifndef LITE3D_ENV_PROBE_PARALAX_CORRECT_REFINE_STEPS
#define LITE3D_ENV_PROBE_PARALAX_CORRECT_REFINE_STEPS       6
#endif

#ifndef LITE3D_ENV_PROBE_PARALAX_CORRECT_MIN_THICKNESS
#define LITE3D_ENV_PROBE_PARALAX_CORRECT_MIN_THICKNESS      0.05
#endif

#ifndef LITE3D_ENV_PROBE_PARALAX_CORRECT_MAX_THICKNESS
#define LITE3D_ENV_PROBE_PARALAX_CORRECT_MAX_THICKNESS      1.0
#endif

#ifndef LITE3D_BASE_AMBIENT_LIGHT
#define LITE3D_BASE_AMBIENT_LIGHT                           vec3(0.0)
#endif

#ifndef LITE3D_CUBE_MAP_UV_SCALE
#define LITE3D_CUBE_MAP_UV_SCALE                            1.0
#endif

#ifndef LITE3D_SKYBOX_UV_SCALE
#define LITE3D_SKYBOX_UV_SCALE                              1.0
#endif

#ifndef LITE3D_ENV_DIFFUSE_STRENGTH
#define LITE3D_ENV_DIFFUSE_STRENGTH                         1.0
#endif

#ifndef LITE3D_SUBSURFACE_SCATTERING
#define LITE3D_SUBSURFACE_SCATTERING                        0.0
#endif

#ifndef LITE3D_BLOOM_STRENGTH
#define LITE3D_BLOOM_STRENGTH                           0.04
#endif

// 0.0f, 2.0f
#ifndef LITE3D_LENSFLARE_GHOSTS_DISPERSAL
#define LITE3D_LENSFLARE_GHOSTS_DISPERSAL               0.71
#endif

// -100.0f, 100.0f
#ifndef LITE3D_LENSFLARE_CHROMATIC_DISTORTION
#define LITE3D_LENSFLARE_CHROMATIC_DISTORTION           -6.5
#endif

#ifndef LITE3D_LENSFLARE_GHOSTS_COUNT
#define LITE3D_LENSFLARE_GHOSTS_COUNT                   4
#endif

// -1.0f, 1.0f
#ifndef LITE3D_LENSFLARE_THRESHOLD_BIAS
#define LITE3D_LENSFLARE_THRESHOLD_BIAS                 -0.9
#endif

// 0.0f, 20.0f
#ifndef LITE3D_LENSFLARE_THRESHOLD_SCALE
#define LITE3D_LENSFLARE_THRESHOLD_SCALE                7.0
#endif

// 0.0f, 50.0f
#ifndef LITE3D_LENSFLARE_GHOSTS_WEIGHT
#define LITE3D_LENSFLARE_GHOSTS_WEIGHT                  27.5
#endif

#ifndef LITE3D_LENSFLARE_GHOSTS_ALPHA
#define LITE3D_LENSFLARE_GHOSTS_ALPHA                   0.1
#endif

#ifndef LITE3D_CHROMATIC_ABERRATION_AMOUNT
#define LITE3D_CHROMATIC_ABERRATION_AMOUNT              0.01
#endif

#ifndef LITE3D_CHROMATIC_ABERRATION_DISTORTION
#define LITE3D_CHROMATIC_ABERRATION_DISTORTION          0.2
#endif

#ifndef LITE3D_GTAO_DIRECTIONS
#define LITE3D_GTAO_DIRECTIONS                          4
#endif

#ifndef LITE3D_GTAO_STEPS
#define LITE3D_GTAO_STEPS                               3
#endif

#ifndef LITE3D_GTAO_RADIUS
#define LITE3D_GTAO_RADIUS                              1.5
#endif

#ifndef LITE3D_GTAO_INTENSITY
#define LITE3D_GTAO_INTENSITY                           1.0
#endif

#ifndef LITE3D_GTAO_FILTER_DEPTH_THRESHOLD
#define LITE3D_GTAO_FILTER_DEPTH_THRESHOLD              0.02
#endif
