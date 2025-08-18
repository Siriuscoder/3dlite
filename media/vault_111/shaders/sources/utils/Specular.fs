uniform sampler2D Specular;
uniform float SpecularFactor;
uniform float MetallicFactor;

vec3 sampleSpecular(vec2 uv)
{
    vec3 specular = texture(Specular, uv).rgb;
    return vec3(specular.r * SpecularFactor, 1.0 - specular.g, specular.r * MetallicFactor);
}