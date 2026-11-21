uniform sampler2D AOMap;
uniform int AOEnabled;

vec4 resolveAO(ivec2 pixelCoord, vec3 geoNormal, vec3 worldCoord)
{
    if (AOEnabled == 0)
    {
        return vec4(geoNormal, 1.0);
    }

    vec4 sampleData = texelFetch(AOMap, pixelCoord, 0);
    return vec4(unpackNormal(sampleData.xy), sampleData.z);
}
