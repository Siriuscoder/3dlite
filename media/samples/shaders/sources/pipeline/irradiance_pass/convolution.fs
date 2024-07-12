#include "samples:shaders/sources/common/version.def"
#include "samples:shaders/sources/common/utils_inc.glsl"

uniform samplerCube SpecularCubeMap;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    vec3 normal = normalize(iuv);
    vec3 irradiance = vec3(0.0);  

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float count = 0.0; 
    for(float phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += texture(SpecularCubeMap, sampleVec).rgb * cos(theta) * sin(theta);
            count++;
        }
    }
    
    irradiance = M_PI * irradiance / count;
    fragColor = vec4(irradiance, 1.0);
}
