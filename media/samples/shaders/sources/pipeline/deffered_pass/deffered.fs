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

void main()
{
    Surface surface = makeSurface(iuv, iwv, iwn, iwt, iwb);
    // Check for alpha clip 
    surfaceAlphaClip(surface.material);

#ifdef LITE3D_BINDLESS_TEXTURE_PIPELINE
    // pack AO and material index to the channel01.w. Material index is being clamped to int16 (65536 materials is limit)
    channel01 = vec4(iwv, packF8AndInt16(surface.ao, surface.materialIndex));
#else 
    // pack AO and environment specular factor to the channel01.w. 
    channel01 = vec4(iwv, pack2xF8(surface.ao, surface.material.envSpecular));
#endif

    channel02 = vec4(surface.normal, surface.material.emission.r);
    channel03 = vec4(surface.material.albedo.rgb, surface.material.emission.g);
    channel04 = vec4(surface.material.specular, surface.material.roughness, surface.material.metallic, surface.material.emission.b);
}