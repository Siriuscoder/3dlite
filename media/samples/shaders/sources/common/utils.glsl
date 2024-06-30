uniform mat4 CameraView; // Main camera view matrix
uniform mat4 CameraProjection; // Main camera projection matrix
uniform float RandomSeed; /* 0.0 - 1.0 */
uniform float Gamma;
uniform float Exposure;
uniform float Contrast;
uniform float Saturation;

#define BAYER_MATRIX_SIZE 4

const float bayerMatrix[BAYER_MATRIX_SIZE * BAYER_MATRIX_SIZE] = float[BAYER_MATRIX_SIZE * BAYER_MATRIX_SIZE](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

bool isNear(float a1, float a2)
{
    return abs(a1 - a2) < FLT_EPSILON;
}

bool isNear(vec3 a1, vec3 a2)
{
    return isNear(a1.x, a2.x) && isNear(a1.y, a2.y) && isNear(a1.y, a2.y); 
}

bool isZero(float a1)
{
    return isNear(a1, 0.0);
}

bool isZero(vec3 a1)
{
    return isNear(a1, vec3(0.0));
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

float doubleSidedNdotV(inout vec3 N, vec3 V)
{
    const float tolerance = -0.12;
    // HdotV
    float NdotV = dot(N, V);
    // Invert normal for double sided materilas 
    if (NdotV < tolerance)
    {
        N *= -1.0;
        NdotV = dot(N, V);
    }
    // Clamp NdotV
    return max(NdotV, FLT_EPSILON);
}

float linearizeDepth(float z, float near, float far)
{
    float zb = z;
    float zn = 2.0 * zb - 1.0;
    return 2.0 * far * near / (near + far - zn * (near - far));
}

// SRGB -> Linear gamma correction
vec3 SRGBToLinear(vec3 color)
{
    vec3 linearLow  = color / 12.92;
    vec3 linearHigh = pow((color + 0.055) / 1.055, vec3(Gamma));
    vec3 isHigh     = step(0.0404482362771082, color);
    return mix(linearLow, linearHigh, isHigh);
}

// Linear -> SRGB gamma correction
vec3 linearToSRGB(vec3 color)
{
    vec3 srgbLow  = color * 12.92;
    vec3 srgbHigh = 1.055 * pow(color, vec3(1.0 / Gamma)) - 0.055;
    vec3 isHigh   = step(0.00313066844250063, color);
    return mix(srgbLow, srgbHigh, isHigh);
}

// Reinhard tone mapping
vec3 reinhardTonemapping(vec3 x)
{
    return x / (x + 1.0);
}

// Exposure tone mapping
vec3 exposureTonemapping(vec3 x)
{
    return 1.0 - exp(-x * Exposure);
}

// Nautilus tone mapping
vec3 nautilusTonemapping(vec3 c)
{

    // Nautilus fit of ACES
    // By Nolram
    
    float a = 2.51;
    float b = 0.03;
    float y = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((c * (a * c + b)) / (c * (y * c + d) + e), 0.0, 1.0);
}

mat4 contrastMatrix()
{
	float t = (1.0 - Contrast) / 2.0;
    return mat4(Contrast, 0, 0, 0,
                0, Contrast, 0, 0,
                0, 0, Contrast, 0,
                t, t, t, 1);
}

mat4 saturationMatrix()
{
    vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
    float oneMinusSat = 1.0 - Saturation;

    vec3 red = vec3(luminance.x * oneMinusSat);
    red += vec3(Saturation, 0, 0);
    
    vec3 green = vec3(luminance.y * oneMinusSat);
    green += vec3(0, Saturation, 0);
    
    vec3 blue = vec3(luminance.z * oneMinusSat);
    blue += vec3(0, 0, Saturation);
    
    return mat4(red,     0,
                green,   0,
                blue,    0,
                0, 0, 0, 1);
}

vec3 ditherBayer(vec3 color)
{
    // Получение позиции пикселя в матрице дизеринга
    int x = int(mod(gl_FragCoord.x, float(BAYER_MATRIX_SIZE)));
    int y = int(mod(gl_FragCoord.y, float(BAYER_MATRIX_SIZE)));
    float ditherValue = bayerMatrix[y * BAYER_MATRIX_SIZE + x] / 16.0;
    
    // Применение дизеринга к цвету
    return color + (ditherValue / 255.0); // Масштабирование для 8-битного цвета
}
