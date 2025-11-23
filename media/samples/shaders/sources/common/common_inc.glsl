#include "samples:shaders/sources/common/structs_inc.glsl"

// The Fresnel-Schlick approximation expects a F0 parameter which is known as the surface 
// reflection at zero incidence or how much the surface reflects if looking directly at the surface. 
// The F0 varies per material and is tinted on metals as we find in large material databases. 
// In the PBR metallic workflow we make the simplifying assumption that most dielectric surfaces 
// look visually correct with a constant F0 of 0.04, while we do specify F0 for metallic surfaces as then 
// given by the albedo value. 
#define LITE3D_BASE_REFLECTION_AT_ZERO_INCIDENCE       0.04
#define LITE3D_MIN_ROUGHNESS                           0.03

//////////// Math utilities
////////////////////////////////////////////////////////////////////////////

#define FLT_EPSILON                             1.192092896e-07F
#define FLT_MAX                                 3.402823466e+38
#define FLT_MIN                                 1.175494351e-38
#define DBL_MAX                                 1.7976931348623158e+308
#define DBL_MIN                                 2.2250738585072014e-308

#define M_PI                                    3.14159265358979323846264
#define PHI                                     1.61803398874989484820459

bool isNear(float a1, float a2);
bool isNear(vec3 a1, vec3 a2);
bool isZero(float a1);
bool isZero(vec3 a1);
bool isValidUV(vec2 uv);
float lerp(float a, float b, float f);
float shlickPow(float a, float b); // Shlick power fast approx a^b = a / (b – a*b + a) for 0 <= a <= 1 
bool hasFlag(uint a, uint flag);
//////////// Noise and random utilities
////////////////////////////////////////////////////////////////////////////

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy);
float noiseInterleavedGradient(vec2 xy);

//////////// Transformations
////////////////////////////////////////////////////////////////////////////
vec3 worldToViewSpacePosition(vec3 vw);
vec3 worldToViewSpaceDirection(vec3 dirw);
vec2 viewPositionToUV(vec3 pos);

mat3 TBN(vec3 normal, vec3 tangent);
mat3 TBN(vec3 normal, vec3 tangent, vec3 btangent);
vec3 calcNormal(vec2 n, mat3 tbn, vec3 normalScale);
vec3 calcNormal(vec3 n, mat3 tbn, vec3 normalScale);
//////////// Other utilities
////////////////////////////////////////////////////////////////////////////
float fadeScreenEdge(vec2 uv);
float doubleSidedNdotV(inout vec3 N, vec3 V);
float linearizeDepth(float z, float near, float far);

//////////// Colour correction utilities 
////////////////////////////////////////////////////////////////////////////
vec3 linearToSRGB(vec3 color);
vec3 SRGBToLinear(vec3 color);
vec3 reinhardTonemapping(vec3 x);
vec3 exponentTonemapping(vec3 x);
vec3 nautilusTonemapping(vec3 x);
vec3 ACESTonemapping(vec3 x);
vec3 contrastColor(vec3 color);
vec3 saturationColor(vec3 color);
vec3 ditherBayer(vec3 color);

//////////// PBR utilities
////////////////////////////////////////////////////////////////////////////
vec3 fresnelSchlickRoughness(float teta, in Material material);
vec3 diffuseFactor(vec3 F, float metallic);
// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness);
// Specular Term GGX
vec3 SpecularGGX(vec3 F, in Material material, in AngularInfo angular);
// Geometry function (Smith's) for IBL intergation
float G_IBL(float NdotV, float NdotL, float roughness);
// Diffuse Term Lambertian (Simple diffuse model)
vec3 DiffuseLambertian(vec3 F, in Material material);
// Sheen 
vec3 Sheen(vec3 F, in Material material, in AngularInfo angular);
// Attenuation
float calcAttenuation(in LightSource source, in AngularInfo angular);

// Importance sample GGX NDF using Hammersley sequence
// This function generates a sample vector towards the alignment of the specular lobe
// using importance sampling. The importance sampling is done using the Hammersley sequence
// which is a low-discrepancy sequence that can be used to generate points that are
// uniformly distributed in the unit square.
//
// The input parameters are:
// Xi: the Hammersley sequence number
// N: the normal vector
// roughness: the material roughness
//
// The output is the sample vector towards the alignment of the specular lobe
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness);

// Hammersley sequence
// This is a low-discrepancy sequence which can be used to generate points that are uniformly distributed
// in the unit square. This is useful for generating points for importance sampling in IBL.
//
// The sequence is generated using the following formulae:
// x_n = (sqrt(2) * n - 1) / (2 * N)
// y_n = (sqrt(3) * (n % N)) / (2 * N)
//
// The sequence is then shuffled by swapping the x and y coordinates of every second point.
vec2 hammersleySequence(uint i, uint N);

vec3 cubeCoordToWorld(ivec3 cubeCoord, vec2 cubemapSize);

//////////// Building structures
Surface makeSurface(vec2 uv, vec3 wv, vec3 wn, vec3 wt, vec3 wb);
Surface restoreSurface(vec2 uv);
void angularInfoInit(inout AngularInfo angular, in Surface surface);
void angularInfoSetLightSource(inout AngularInfo angular, in Surface surface, in LightSource source);
void angularInfoCalcAngles(inout AngularInfo angular, in Surface surface);
//////////// Clip Functions
void surfaceAlphaClip(in Material material);
void surfaceAlphaClip(vec2 uv);

