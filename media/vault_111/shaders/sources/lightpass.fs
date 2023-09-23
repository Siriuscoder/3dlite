uniform sampler2D Albedo;
uniform sampler2D Specular;
uniform sampler2D VwMap;
uniform sampler2D NwMap;
uniform samplerCube Environment;

#ifdef USE_SHADOWMAP
uniform sampler2DShadow ShadowMap;
uniform mat4 ShadowMatrix;
#endif

uniform vec3 eye;

struct lightSource 
{
    int enabled;
    int type;
    vec3 position;
    vec3 diffuse;
    vec3 direction;
    float influenceDistance;
    float attenuationContant;
    float attenuationLinear;
    float attenuationQuadratic;
    float innercone;
    float outercone;
};

uniform lightSource Light;

in vec2 iuv;

void main()
{
    vec4 albedo = texture(Albedo, iuv);
    gl_FragColor = vec4(albedo.rgb, 1.0);
}