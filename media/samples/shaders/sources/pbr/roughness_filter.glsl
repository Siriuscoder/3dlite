float filterRoughness(vec3 N, float roughness)
{
    vec3 dndx = dFdx(N);
    vec3 dndy = dFdy(N);

    float variance = dot(dndx, dndx) + dot(dndy, dndy);

    float kernel = min(variance * 0.05, 0.04);
    float alpha = roughness * roughness;

    float r = sqrt(sqrt(alpha * alpha + kernel));
    return clamp(r, LITE3D_MIN_ROUGHNESS, 1.0);
}
