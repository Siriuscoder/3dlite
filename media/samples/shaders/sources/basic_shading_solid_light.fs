uniform vec4 Albedo;

in vec2 uv;
in vec3 wn;

out vec4 fragcolor;

void main()
{
    vec3 lightDir = vec3(0.0, 1.0, 1.0);
    float nDotL = clamp(dot(wn, lightDir), 0.2, 1.0);
    fragcolor = Albedo * nDotL;
}