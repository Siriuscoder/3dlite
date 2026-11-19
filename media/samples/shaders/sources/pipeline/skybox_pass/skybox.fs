uniform samplerCube Skybox;
uniform float EmissionStrength;

in vec3 iuv;
out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(Skybox, iuv * LITE3D_SKYBOX_UV_SCALE).rgb * EmissionStrength, 1.0);
}