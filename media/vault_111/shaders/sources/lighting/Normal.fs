#include "samples:shaders/sources/common/version.def"

uniform sampler2D Normal;

vec3 GetFixedWorldNormal(mat3 tbn, vec2 iuv)
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
    vec3 nt = texture(Normal, iuv).rgb;
    // Flip Y channel to fix model texcoords flip while export models (FlipUV)
    nt.y = 1.0 - nt.y;
    // put normal in [-1,1] range in tangent space
    nt = 2.0 * clamp(nt, 0.0, 1.0) - 1.0;
    // Refix Z (may be missing)
    nt.z = sqrt(1.0 - dot(nt.xy, nt.xy));
    // trasform normal to world space using common TBN
    return normalize(tbn * nt);
}
