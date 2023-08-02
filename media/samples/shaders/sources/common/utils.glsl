const float eps = 0.000001;

bool fnear(float a1, float a2)
{
    return abs(a1 - a2) < eps;
}

bool fnear(vec3 a1, vec3 a2)
{
    return all(equal(a1, a2));
}


bool fiszero(float a1)
{
    return fnear(a1, 0.0);
}

bool fiszero(vec3 a1)
{
    return fnear(a1, vec3(0.0));
}
