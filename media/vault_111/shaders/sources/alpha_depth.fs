uniform sampler2D Albedo;

vec4 getAlbedo(vec2 uv)
{
    return texture(Albedo, uv);
}
