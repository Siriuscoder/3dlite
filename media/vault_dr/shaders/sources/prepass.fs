#include "samples:shaders/sources/common/utils_inc.glsl"
#include "samples:shaders/sources/lighting/lighting_inc.glsl"

uniform sampler2D diffuse;
uniform sampler2D normals;
uniform vec3 eye;

#ifdef CALC_ILLUM
uniform sampler2D glow;
#endif

#ifdef CALC_PARALLAX
uniform sampler2D depth;
uniform float pscale;
#endif

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;
in vec3 wnorm;

out vec4 fragColor;

const vec3 fogColor = vec3(0.0, 0.95, 0.89);
const vec3 ambient = vec3(0.03, 0.03, 0.03);
const float wrapAroundFactor = 0.04;
#ifdef GLASS
const float specPower = 180.0;
#else
const float specPower = 40.0;
#endif

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

vec3 fogFunc(vec3 frag, vec3 fc)
{
    const float density = 0.0003;
    const float LOG2 = 1.442695;

    float zfactor = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = clamp(exp2(-density * density * zfactor * zfactor * LOG2), 0.0, 1.0);
    return mix(fc, frag, fogFactor);
}

void main()
{
    vec2 tc = iuv;
#ifdef CALC_PARALLAX
    // Offset texture coordinates with Parallax Mapping
    vec3 tanViewDir = normalize(transpose(itbn) * (eye - ivv));
    tc = ParallaxMapping(iuv, tanViewDir);
#endif

    // sampling diffuse color 
    vec4 fragDiffuse = texture(diffuse, tc);

#ifdef CALC_ILLUM
    // sampling glow texture and check colors
    vec3 fragGlow = texture(glow, tc).rgb;
    if (!fiszero(fragGlow))
    {
        fragColor = vec4(fogFunc(fragGlow + (fragDiffuse.rgb/3), fogColor), fragDiffuse.a);
        return;
    }
#endif

    // sampling normal from normal map
    vec4 nval = texture(normals, tc);
    nval.y = 1.0 - nval.y;
    // put normal in [-1,1] range in tangent space 
    // and trasform normal to world space 
    vec3 nw = normalize(itbn * normalize(2.0 * nval.xyz - 1.0));

    // fix bad normals
    if (fiszero(nw))
        nw = wnorm;

    vec3 linearSpec;
    vec3 linear = calc_lighting(ivv, nw,
        eye, nval.w * 2, wrapAroundFactor, specPower, linearSpec);

#ifdef GLASS
    fragDiffuse.a = mix(fragDiffuse.a, 1.0, length(linearSpec.rgb));
#endif

    /* calculate fog factor */
    fragColor = vec4(fogFunc((ambient + linear) * fragDiffuse.rgb, fogColor), fragDiffuse.a);
}