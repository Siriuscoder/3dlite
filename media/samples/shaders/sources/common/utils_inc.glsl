#define M_PI 3.1415926535897932384626433832795
#define FLT_EPSILON 1.192092896e-07F
#define PHI 1.61803398874989484820459

bool isNear(float a1, float a2);
bool isNear(vec3 a1, vec3 a2);

bool isZero(float a1);
bool isZero(vec3 a1);
bool isValidUV(vec2 uv);

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy);
float noiseInterleavedGradient(vec2 xy);

float lerp(float a, float b, float f);

vec3 worldToViewSpacePosition(vec3 vw);
vec3 worldToViewSpaceDirection(vec3 dirw);
vec2 viewPositionToUV(vec3 pos);

float fadeScreenEdge(vec2 uv);
float doubleSidedNdotV(inout vec3 N, vec3 V);
