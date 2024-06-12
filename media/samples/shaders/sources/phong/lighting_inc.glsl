#define LITE3D_LIGHT_UNDEFINED          0.0
#define LITE3D_LIGHT_POINT              1.0
#define LITE3D_LIGHT_DIRECTIONAL        2.0
#define LITE3D_LIGHT_SPOT               3.0

vec3 calc_lighting(vec3 fragPos, vec3 fragNormal, vec3 eye, float specularFactor, 
    float wrapAroundFactor, float specPower, inout vec3 linearSpec);

vec3 phong_blinn_single(float type, vec3 lightDir, vec3 eyeDir, vec3 diffuse, 
    vec3 normal, vec3 spotDirection, vec2 spotFactor, vec3 attenuation, 
    float specularFactor, float wrapAroundFactor, float specPower, inout vec3 linearSpec);