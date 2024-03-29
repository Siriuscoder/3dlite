#include "samples:shaders/sources/lighting/lighting_inc.glsl"

uniform sampler2D diffuse;
#ifdef NORMAL_MAP
uniform sampler2D normals;
#endif
#ifdef SPECULAR_MAP
uniform sampler2D specular;
#endif
uniform vec3 eye;
uniform sampler2DShadow shadowmap;

in vec2 iuv;
in vec3 ivv;
in mat3 itbn;
in vec3 wnorm;
in vec4 svv;

out vec4 fragColor;

const vec3 ambient = vec3(0.33, 0.33, 0.33);
const float wrapAroundFactor = -0.04;
const float specPower = 35.0;
const float shadowBias = 0.0000;

float shadow_PCF()
{
    vec3 psf = svv.xyz / svv.w;
    // transform the NDC coordinates to the range [0,1]
    psf = psf * 0.5 + 0.5;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowmap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 psf_sft = psf.xy + (vec2(x, y) * texelSize);
            if (psf_sft.x < 0.0 || psf_sft.x > 1.0 || psf_sft.y < 0.0 || psf_sft.y > 1.0 || psf.z > 1.0)
                continue;

            shadow += texture(shadowmap, vec3(psf_sft, psf.z - shadowBias));
        }
    }

    shadow /= 9.0;
    return shadow;
}

bool vec3zero(vec3 vec)
{
    float prec = 0.000001;
    return ((1.0-step(prec, vec.x)) * (1.0-step(prec, vec.y)) * (1.0-step(prec, vec.z))) == 1.0;
}

void main()
{
    vec2 tc = iuv;
    vec3 nw = wnorm;
    vec3 spec = vec3(0.0, 0.0, 0.0);
    vec3 linear = vec3(0.0, 0.0, 0.0);

    // sampling diffuse color 
    vec4 fragDiffuse = texture(diffuse, tc);
    
    float shadowFactor = shadow_PCF();
    if (shadowFactor > 0.00001)
    {

#ifdef NORMAL_MAP
        // sampling normal from normal map
        vec4 nval = texture(normals, tc);
        // put normal in [-1,1] range in tangent space 
        // and trasform normal to world space 
        nw = normalize(itbn * normalize(2*nval.rgb-1));
    
        // fix bad normals
        if (vec3zero(nw))
            nw = wnorm;
#endif

#ifdef SPECULAR_MAP
        spec = texture(specular, tc).xyz;
#endif

        vec3 linearSpec = vec3(0.0);
        linear = calc_lighting(ivv, nw, eye, spec.x/1.7, 
            wrapAroundFactor, specPower, linearSpec);
    }

    vec3 final = ambient + (shadowFactor * linear);
    fragColor = vec4(final * fragDiffuse.rgb, 1.0);
}