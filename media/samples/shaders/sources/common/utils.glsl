uniform mat4 CameraView; // Main camera view matrix
uniform mat4 CameraProjection; // Main camera projection matrix
uniform float RandomSeed; /* 0.0 - 1.0 */

bool fnear(float a1, float a2)
{
    return abs(a1 - a2) < FLT_EPSILON;
}

bool fnear(vec3 a1, vec3 a2)
{
    return fnear(a1.x, a2.x) && fnear(a1.y, a2.y) && fnear(a1.y, a2.y); 
}

bool fiszero(float a1)
{
    return fnear(a1, 0.0);
}

bool fiszero(vec3 a1)
{
    return fnear(a1, vec3(0.0));
}

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy)
{
    return fract(tan(distance(xy * PHI, xy) * RandomSeed) * xy.x);
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

float noiseInterleavedGradient(vec2 xy)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(xy * RandomSeed, magic.xy)));
}

vec3 worldToViewSpacePosition(vec3 posw)
{
    vec4 pos = CameraView * vec4(posw, 1.0);
    return pos.xyz / pos.w;
}

vec3 worldToViewSpaceDirection(vec3 vw)
{
    vec4 dir = CameraView * vec4(vw, 0.0);
    return normalize(dir.xyz);
}

vec2 viewPositionToUV(vec3 pos)
{
    vec4 uv = CameraProjection * vec4(pos, 1.0);
    uv /= uv.w;                 // perspective divide
    return uv.xy * 0.5 + 0.5;   // transform to range 0.0 - 1.0 
}

float fadeScreenEdge(vec2 uv)
{
    vec2 fade = max(vec2(0.0), 12.0 * abs(uv - 0.5) - 5.0);
    return clamp(1.0 - dot(fade, fade), 0.0, 1.0);
}

bool isValidUV(vec2 uv)
{
    return !(uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0);
}