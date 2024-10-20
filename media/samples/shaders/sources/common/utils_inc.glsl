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
bool hasFlag(int a, int flag);
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
vec3 exposureTonemapping(vec3 x);
vec3 nautilusTonemapping(vec3 c);
mat4 contrastMatrix();
mat4 saturationMatrix();
vec3 ditherBayer(vec3 color);

//////////// PBR utilities
////////////////////////////////////////////////////////////////////////////
vec3 fresnelSchlickRoughness(float teta, vec3 albedo, vec3 specular);
vec3 diffuseFactor(vec3 F, float metallic);
// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness);
// Geometry function (Schlick-Beckmann, Schlick-GGX)
float GGX(float NdotV, float roughness);
// Geometry function (Smith's)
float G(float NdotV, float NdotL, float roughness);

// Common structs
#include "samples:shaders/sources/common/structs_inc.glsl"
