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