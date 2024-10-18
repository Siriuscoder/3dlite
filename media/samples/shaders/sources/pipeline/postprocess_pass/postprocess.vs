layout(location = 0) in vec2 vertex;

uniform mat4 screenMatrix;
uniform vec3 ScreenResolution;

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
    out vec2 rgbM); 

void main()
{
    iuv = vertex;

    texcoords(iuv * ScreenResolution.xy, ScreenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM);
    gl_Position = screenMatrix * vec4(vertex.xy, 0.0, 1.0);
}