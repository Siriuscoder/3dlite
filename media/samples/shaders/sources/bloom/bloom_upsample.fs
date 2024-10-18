// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
uniform sampler2D Source;
uniform float BloomRadius;

in vec2 iuv;
out vec3 upsample;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = BloomRadius;
    float y = BloomRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(Source, vec2(iuv.x - x, iuv.y + y)).rgb;
    vec3 b = texture(Source, vec2(iuv.x,     iuv.y + y)).rgb;
    vec3 c = texture(Source, vec2(iuv.x + x, iuv.y + y)).rgb;

    vec3 d = texture(Source, vec2(iuv.x - x, iuv.y)).rgb;
    vec3 e = texture(Source, vec2(iuv.x,     iuv.y)).rgb;
    vec3 f = texture(Source, vec2(iuv.x + x, iuv.y)).rgb;

    vec3 g = texture(Source, vec2(iuv.x - x, iuv.y - y)).rgb;
    vec3 h = texture(Source, vec2(iuv.x,     iuv.y - y)).rgb;
    vec3 i = texture(Source, vec2(iuv.x + x, iuv.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
}
