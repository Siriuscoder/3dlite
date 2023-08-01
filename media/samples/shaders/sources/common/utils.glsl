#include "samples:shaders/sources/common/version.def"

const float eps = 0.0001;

bool fnear(float a1, float a2)
{
    return abs(a1 - a2) < eps;
}

bool fiszero(float a1)
{
    return fnear(a1, 0.0);
}
