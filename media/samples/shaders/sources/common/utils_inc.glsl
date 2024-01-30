#define M_PI 3.1415926535897932384626433832795
#define FLT_EPSILON 1.192092896e-07F
#define PHI 1.61803398874989484820459

bool fnear(float a1, float a2);
bool fnear(vec3 a1, vec3 a2);

bool fiszero(float a1);
bool fiszero(vec3 a1);

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy, float seed);
float lerp(float a, float b, float f);
