#include "samples:shaders/sources/common/common_inc.glsl"

layout(location = 0) out vec4 channel01;
layout(location = 1) out vec4 channel02;
layout(location = 2) out vec4 channel03;
layout(location = 3) out vec4 channel04;

in vec2 iuv;    // UVs
in vec3 iwv;    // world-space position
in vec3 iwn;    // world-space normal
in vec3 iwt;    // world-space tangent
in vec3 iwb;    // world-space bitangent

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}
#endif

/*
 * GBuffer layout: 4 x RGBA32F
 *
 * +-----------+-------------------+-------------------+-------------------+-------------------+
 * | Channel   | X                 | Y                 | Z                 | W                 |
 * +-----------+-------------------+-------------------+-------------------+-------------------+
 * | channel01 | iwv.x             | iwv.y             | iwv.z             | MatID(24)         |
 * | channel02 | normal.x          | normal.y          | normal.z          | emission.r        |
 * | channel03 | albedo.rg (2x12)  | albedo.b(12)+S(12)| geoNormal (Oct24) | emission.g        |
 * | channel04 | R(12)+M(12)       | AO / reserved     | reserved          | emission.b        |
 * +-----------+-------------------+-------------------+-------------------+-------------------+
 *
 * Packing:
 *   albedo.rg  : 2 x UNORM12 -> FP32
 *   albedo.b/S : 2 x UNORM12 -> FP32
 *   R/M        : 2 x UNORM12 -> FP32
 *   geoNormal  : Octahedral 2 x UNORM12 -> FP32
 *
 * S = Specular, R = Roughness, M = Metallic
 *
 * All new packed values use numeric uint-to-float conversion (no NaN).
 * Packed channels must be read without texture filtering.
 */

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);
    // Check for alpha clip 
    surfaceAlphaClip(surface.material);

    // channel01 | iwv.x             | iwv.y             | iwv.z             | MatID(24) |
#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    // pack AO and material index to the channel01.w. Material index is being clamped to int16 (65536 materials is limit)
    channel01 = vec4(iwv, float(surface.materialIndex));
#else 
    // pack environment diffuse factor and environment specular factor to the channel01.w. 
    channel01 = vec4(iwv, pack2xF8(vec2(surface.material.envDiffuse, surface.material.envSpecular)));
#endif

    // channel02 | normal.x          | normal.y          | normal.z          | emission.r        |
    channel02 = vec4(surface.normal, surface.material.emission.r);

    // channel03 | albedo.rg (2x12)  | albedo.b(12)+S(12)| geoNormal (Oct24) | emission.g        |
    channel03 = vec4(
        pack2xF12(surface.material.albedo.rg),
        pack2xF12(vec2(surface.material.albedo.b, surface.material.specular)),
        packNormal2xF12(surface.geoNormal),
        surface.material.emission.g);

    // channel04 | R(12)+M(12)       | reserved          | reserved          | emission.b        |
    channel04 = vec4(
        pack2xF12(vec2(surface.material.roughness, surface.material.metallic)),
        surface.ao, 
        0, // reserved 
        surface.material.emission.b);
}
