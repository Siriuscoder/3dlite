#include "samples:shaders/sources/common/version.def"

uniform sampler2D Albedo;
uniform sampler2D Normal;
uniform sampler2D Specular;

layout(location = 0) out vec4 fragWCoord;
layout(location = 1) out vec4 fragWNormal;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec4 fragSpecular;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;

vec3 GetFixedWorldNormal()
{
    // There are different DDS loaders for Fallout4 or Blender loads normals 
    // directly in GRB formal but native DDS viewer show image correctly in RGB format, also 
    // Fallout4 or Blender loaders flip image by Y axis while load. RGBA format, such as Albedo has
    // correctly channels everywhere. The reason of this is not known for me. DevIL loads DDS channels 
    // as DDS viewer shows. So, i am using common TBN matrix and it is require to 
    // swap normal XY channels. (One of the other solution to use BTN matrix).  
    // By reason that DevIL is not flipping image while load i should Flip UV while exporting meshes to
    // engine native format and normal Y channel has become inverted. 
    // This behavour is not been expected for me.. So lets try to fix this.

    // sampling normal from normal map with swap XY
    vec3 nt = texture(Normal, iuv).grb;
    // Flip Y channel to fix model texcoords flip while export models (FlipUV)
    nt.y = 1.0 - nt.y;
    // put normal in [-1,1] range in tangent space
    nt = normalize(2.0 * nt - 1.0);
    // Refix Z (may be missing)
    nt.z = sqrt(1.0 - dot(nt.xy, nt.xy));
    // trasform normal to world space using common TBN
    return normalize(itbn * nt);
}

void main()
{
    // sampling albedo 
    vec4 albedo = texture(Albedo, iuv);
    // sampling specular
    vec4 specular = texture(Specular, iuv);
    // Transform Specular, Roughness, Metalic 
    specular = vec4(specular.g,
        clamp(1.0 - specular.r, 0.0, 1.0), 
        clamp(1.0 - specular.b, 0.0, 1.0), 
        0.0);

    fragWCoord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    fragWNormal = vec4(GetFixedWorldNormal(), 0.0);
    fragAlbedo = vec4(albedo.rgb, 0.0);
    fragSpecular = specular;
}