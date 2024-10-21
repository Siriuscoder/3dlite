uniform sampler2D Albedo;
uniform float SpecularAmbientFactor;

vec3 specularTmp = vec3(0.0);

vec3 sampleSpecular(vec2 iuv);
vec3 sampleNormal(vec2 iuv, mat3 tbn);

vec4 getAlbedo(vec2 uv)
{
    vec4 albedo = texture(Albedo, uv);
    specularTmp = sampleSpecular(uv);

    float alpha = mix(albedo.a, 1.0, specularTmp.y);
    return vec4(albedo.rgb, alpha);
}

vec3 getEmission(vec2 uv)
{
    return vec3(0.0);
}

vec3 getNormal(vec2 uv, mat3 tbn)
{
    return sampleNormal(uv, tbn);
}

vec3 getSpecular(vec2 uv)
{
    return vec3(specularTmp.x, specularTmp.y / 10.0, specularTmp.z);
}

float getAmbientOcclusion(vec2 uv)
{
    return 1.0;
}

float getSpecularAmbient(vec2 uv)
{
    return SpecularAmbientFactor;
}