
#include "samples:shaders/sources/common/common_inc.glsl"

uniform sampler2D ltcLut1;
uniform sampler2D ltcLut2;

const float LTC_LUT_SIZE  = 64.0;
const float LTC_LUT_SCALE = (LTC_LUT_SIZE - 1.0)/LTC_LUT_SIZE;
const float LTC_LUT_BIAS  = 0.5/LTC_LUT_SIZE;

vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

vec3 LTC_EvaluateQuad(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4])
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon 
    vec3 L[4];
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    // integrate
    float sum = 0.0;
    vec3 dir = points[0].xyz - P;
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);
    if (behind)
        return vec3(0.0);

    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    vec3 vsum = vec3(0.0);

    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    float len = length(vsum);
    float z = vsum.z/len;

    vec2 uv = vec2(z*0.5 + 0.5, len);
    uv = uv * LTC_LUT_SCALE + LTC_LUT_BIAS;

    float scale = texture(ltcLut2, uv).w;
    sum = len * scale;

    return vec3(sum);
}

vec3 LTC(in LightSource source, in Surface surface, in AngularInfo angular)
{
    // Calc the rect area of the light source
    vec3 directionX = cross(source.direction.xyz, source.directionUP.xyz);
    vec3 ex = source.areaWidth / 2.0 * directionX.xyz;
    vec3 ey = source.areaHeight / 2.0 * source.directionUP.xyz;

    vec3 points[4] = {
        source.position.xyz - ex - ey,
        source.position.xyz + ex - ey,
        source.position.xyz + ex + ey,
        source.position.xyz - ex + ey
    };

    vec2 uv = vec2(surface.material.roughness, sqrt(1.0 - angular.NdotV));
    uv = uv * LTC_LUT_SCALE + LTC_LUT_BIAS;

    vec4 t1 = texture(ltcLut1, uv);
    vec4 t2 = texture(ltcLut2, uv);
    
    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(  0,  1,    0),
        vec3(t1.z, 0, t1.w)
    );

    vec3 spec = vec3(0.0), diff = vec3(0.0);
    if (hasFlag(source.flags, LITE3D_LIGHT_RECT_AREA))
    {
        // Specular quad term
        spec = LTC_EvaluateQuad(surface.normal, angular.viewDir, surface.wv, Minv, points);
        // Diffuse quad term
        diff = LTC_EvaluateQuad(surface.normal, angular.viewDir, surface.wv, mat3(1), points);
    }

    vec3 radiance = source.diffuse.rgb * source.radiance * surface.ao;
    vec3 kD = diffuseFactor(angular.F, surface.material.metallic);
    return radiance * (diff * surface.material.albedo.rgb * kD + spec * angular.F * surface.material.specular);
}
