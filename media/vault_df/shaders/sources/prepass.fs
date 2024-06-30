#include "samples:shaders/sources/common/utils_inc.glsl"

uniform sampler2D diffuse;
uniform sampler2D normals;
#ifdef CALC_ILLUM
uniform sampler2D glow;
#endif

#ifdef CALC_PARALLAX
uniform sampler2D depth;
uniform vec3 eye;
uniform float pscale;
#endif

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;
in vec3 wnorm;

layout(location = 0) out vec4 coord;
layout(location = 1) out vec4 norm;
layout(location = 2) out vec4 color;

#ifdef CALC_PARALLAX
vec2 ParallaxMapping(vec2 tc, vec3 viewDir)
{ 
    // number of depth layers
    const float numLayers = 10;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 deltaTexCoords = viewDir.xy * pscale / numLayers;

    vec2 currentTexCoords = tc;
    float currentDepthMapValue = texture(depth, currentTexCoords).r;
  
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depth, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    // Parallax Occlusion Mapping
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depth, prevTexCoords).r - currentLayerDepth + layerDepth;
     
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}
#endif

void main()
{
    vec2 tc = iuv;
#ifdef CALC_PARALLAX
    // Offset texture coordinates with Parallax Mapping
    vec3 tanViewDir = normalize(transpose(itbn) * (eye - ivv));
    tc = ParallaxMapping(iuv, tanViewDir);
#endif

    // sampling diffuse color 
    vec4 fragDiffuse = vec4(texture(diffuse, tc).rgb, 0.0);

#ifdef CALC_ILLUM
    // sampling glow texture and check colors
    vec3 fragGlow = texture(glow, tc).rgb;
    if (!isZero(fragGlow))
    {
        fragDiffuse = vec4(fragGlow, 1.0);
    }
#endif

    // sampling normal from normal map
    vec4 nval = texture(normals, tc);
    nval.y = 1.0 - nval.y;
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2.0 * nval.xyz - 1.0));

    // fix bad normals
    if (isZero(nw))
        nw = wnorm;

    coord = vec4(ivv, gl_FragCoord.z / gl_FragCoord.w);
    norm = vec4(nw, nval.a);
    color = fragDiffuse;
}