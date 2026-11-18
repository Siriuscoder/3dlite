// Возвращает разницу:
// < 0  -> candidate point находится перед depth surface
// > 0  -> candidate point находится за depth surface
float probeDepthError(in Material material, vec3 probePos, vec3 rayOrigin, vec3 rayDir, float t, float bias, int probeIndex)
{
    vec3 p = rayOrigin + rayDir * t;
    vec3 probeToPoint = p - probePos;

    float candidateAxisDepth = max(
        max(abs(probeToPoint.x), abs(probeToPoint.y)),
        abs(probeToPoint.z)
    );

    if (candidateAxisDepth < 1e-6)
        return -1.0;

    float depth = sampleProbeDepth(material, probeToPoint, probeIndex);
    float linearDepth = linearizeDepth(depth, LITE3D_ENV_PROBE_ZNEAR, LITE3D_ENV_PROBE_ZFAR);

    return candidateAxisDepth - (linearDepth + bias);
}

vec4 probeParallaxCorrect(in Surface surface, vec3 probePos, vec3 R, float probeDistance, int probeIndex)
{
    float bias = LITE3D_ENV_PROBE_PARALAX_CORRECT_MIN_THICKNESS + 0.01 * probeDistance;
    vec3 worldPos = surface.wv;
    
    // Пробный поиск рядом
    float tPrev = max(LITE3D_ENV_PROBE_ZNEAR, 0.01);
    float ePrev = probeDepthError(surface.material, probePos, worldPos, R, tPrev, bias, probeIndex);

    // Не начинаем поиск, если стартовая
    // точка уже находится за depth surface.
    if (ePrev > 0.0)
        return vec4(R, 1.0);

    float tMin = 0.0;
    float tMax = 0.0;

    bool hitFound = false;

    const float stepSize = LITE3D_ENV_PROBE_ZFAR / 
        float(LITE3D_ENV_PROBE_PARALAX_CORRECT_COARSE_STEPS);

    // Грубый поиск пересечения
    for (int i = 1; i <= LITE3D_ENV_PROBE_PARALAX_CORRECT_COARSE_STEPS; ++i)
    {
        float t = float(i) * stepSize;
        float e = probeDepthError(surface.material, probePos, worldPos, R, t, bias, probeIndex);

        if (ePrev <= 0.0 && e > 0.0)
        {
            tMin = tPrev;
            tMax = t;
            hitFound = true;
            break;
        }

        tPrev = t;
        ePrev = e;
    }

    // Пересечение не найдено, значит упираемся в skybox, коррекция не нужна
    if (!hitFound)
        return vec4(R, 1.0);

    // Бинарное уточнение
    for (int i = 0; i < LITE3D_ENV_PROBE_PARALAX_CORRECT_REFINE_STEPS; ++i)
    {
        float tMid = 0.5 * (tMin + tMax);
        float e = probeDepthError(surface.material, probePos, worldPos, R, tMid, bias, probeIndex);

        if (e < 0.0)
            tMin = tMid;
        else
            tMax = tMid;
    }

    // Оценка плавности границы, корреция веса пробы для которой пересечение недостоверно
    float eMin = probeDepthError(surface.material, probePos, worldPos, R, tMin, bias, probeIndex);
    float eMax = probeDepthError(surface.material, probePos, worldPos, R, tMax, bias, probeIndex);
    float discontinuity = abs(eMax - eMin);

    float tHit = 0.5 * (tMin + tMax);
    vec3 hitPos = worldPos + R * tHit;

    float confidence = 1.0 - smoothstep(
        LITE3D_ENV_PROBE_PARALAX_CORRECT_MIN_THICKNESS,
        LITE3D_ENV_PROBE_PARALAX_CORRECT_MAX_THICKNESS,
        discontinuity
    );

    return vec4(normalize(hitPos - probePos), confidence);
}
