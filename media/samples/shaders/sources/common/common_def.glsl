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

#ifndef LITE3D_SHADOW_VSM_MIN_VARIANCE
#define LITE3D_SHADOW_VSM_MIN_VARIANCE                      0.000001
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
