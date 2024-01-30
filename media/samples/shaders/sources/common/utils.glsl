bool fnear(float a1, float a2)
{
    return abs(a1 - a2) < FLT_EPSILON;
}

bool fnear(vec3 a1, vec3 a2)
{
    return fnear(a1.x, a2.x) && fnear(a1.y, a2.y) && fnear(a1.y, a2.y); 
}

bool fiszero(float a1)
{
    return fnear(a1, 0.0);
}

bool fiszero(vec3 a1)
{
    return fnear(a1, vec3(0.0));
}

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method
float goldNoise(vec2 xy, float seed)
{
    return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}
