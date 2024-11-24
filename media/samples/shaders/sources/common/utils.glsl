uniform mat4 CameraView; // Main camera view matrix
uniform mat4 CameraProjection; // Main camera projection matrix
uniform float RandomSeed; /* 0.0 - 1.0 */
uniform float Gamma;
uniform float Exposure;
uniform float Contrast;
uniform float Saturation;

#define BAYER_MATRIX_SIZE                       4

// The Fresnel-Schlick approximation expects a F0 parameter which is known as the surface 
// reflection at zero incidence or how much the surface reflects if looking directly at the surface. 
// The F0 varies per material and is tinted on metals as we find in large material databases. 
// In the PBR metallic workflow we make the simplifying assumption that most dielectric surfaces 
// look visually correct with a constant F0 of 0.04, while we do specify F0 for metallic surfaces as then 
// given by the albedo value. 
#define BASE_REFLECTION_AT_ZERO_INCIDENCE       0.04

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
    return isNear(a1.x, a2.x) && isNear(a1.y, a2.y) && isNear(a1.z, a2.z); 
}

bool isZero(float a1)
{
    return isNear(a1, 0.0);
}

bool isZero(vec3 a1)
{
    return isNear(a1, vec3(0.0));
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

float shlickPow(float a, float b)
{
    return a / (b - a * b + a);
}

bool hasFlag(uint a, uint flag)
{
    return (a & flag) == flag;
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

mat3 TBN(vec3 normal, vec3 tangent)
{
    normal = normalize(normal);
    // re-orthogonalize T with respect to N
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);
    // TBN transforms vector from tangent space to world space 
    return mat3(tangent, bitangent, normal);
}

mat3 TBN(vec3 normal, vec3 tangent, vec3 btangent)
{
    return mat3(
        normalize(tangent),
        normalize(btangent),
        normalize(normal)
    );
}

vec3 calcNormal(vec2 n, mat3 tbn, vec3 normalScale)
{
    // put normal in [-1,1] range in tangent space
    n = 2.0 * clamp(n, 0.0, 1.0) - 1.0;
    // Refix Z (may be missing)
    float z = sqrt(1.0 - dot(n, n));
    // trasform normal to world space using common TBN
    return normalize(tbn * normalize(vec3(n, z) * normalScale));
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

#ifdef LITE3D_FRAGMENT_SHADER

vec3 ditherBayer(vec3 color)
{
    // Получение позиции пикселя в матрице дизеринга
    int x = int(mod(gl_FragCoord.x, float(BAYER_MATRIX_SIZE)));
    int y = int(mod(gl_FragCoord.y, float(BAYER_MATRIX_SIZE)));
    float ditherValue = bayerMatrix[y * BAYER_MATRIX_SIZE + x] / 16.0;
    
    // Применение дизеринга к цвету
    return color + (ditherValue / 255.0); // Масштабирование для 8-битного цвета
}

#endif

// Fresnel equation (Schlick)
vec3 fresnelSchlickRoughness(float teta, vec3 albedo, vec3 specular)
{
    // Calculate F0 coeff (metalness)
    vec3 F0 = vec3(BASE_REFLECTION_AT_ZERO_INCIDENCE);
    F0 = mix(F0, albedo, specular.z);

    vec3 F = F0 + (max(vec3(1.0 - specular.y), F0) - F0) * pow(clamp(1.0 - teta, 0.0, 1.0), 5.0);
    return clamp(F * specular.x, 0.0, 1.0);
}

vec3 diffuseFactor(vec3 F, float metallic)
{
    // PBR модель строится на принципе сохранения энергии и по этому энергия поглощенного и отраженного 
    // света в суммме не могут быть больше чем энергия падающего луча от источника света  
    // F - Кофф Френеля по сути определяет отраженную часть света, поэтому kD - Кофф рассеяного света
    // вычисляется просто kD = 1 - f , но с поправкой на металл/диэлектрик. Металл хуже рассеивает свет.
    vec3 kD = 1.0 - F;
    return kD * (1.0 - metallic);
}

// Normal distribution function (Trowbridge-Reitz GGX)
float NDF(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
	
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;
	
    return nom / denom;
}

// Geometry function (Schlick-Beckmann, Schlick-GGX)
float GGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

// Geometry function (Smith's)
float G(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GGX(NdotV, roughness);
    float ggx1  = GGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}