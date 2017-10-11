in vec2 ivertex;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 screenResolution;

out vec2 iuv;
out vec2 irgbNW;
out vec2 irgbNE;
out vec2 irgbSW;
out vec2 irgbSE;
out vec2 irgbM;

// needed for compute fxaa 
void texcoords(vec2 fragCoord, vec2 resolution,
    out vec2 rgbNW, out vec2 rgbNE,
    out vec2 rgbSW, out vec2 rgbSE,
    out vec2 rgbM) 
{
    vec2 inverseVP = 1.0 / resolution.xy;
    rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
    rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
    rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
    rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
    rgbM = vec2(fragCoord * inverseVP);
}

void main()
{
    iuv = ivertex;
    gl_Position = projectionMatrix * viewMatrix * vec4(ivertex.xy, 0.0, 1.0);

    texcoords(iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM);
}