#include "samples:shaders/sources/common/version.def"

layout(location = 0) in vec2 ivertex;

uniform mat4 screenMatrix;
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
    out vec2 rgbM);

void main()
{
    iuv = ivertex;
    gl_Position = screenMatrix * vec4(ivertex.xy, 0.0, 1.0);

    texcoords(iuv * screenResolution.xy, screenResolution.xy, irgbNW, irgbNE, irgbSW, irgbSE, irgbM);
}