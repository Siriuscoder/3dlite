uniform sampler2DArray GBuffer;

float SSS(vec3 P, in AngularInfo angular)
{
    // Compute ray position and direction (in view-space)
    vec3 rayPos = worldToViewSpacePosition(P);
    vec3 rayDir = worldToViewSpaceDirection(angular.lightDir);

    float distanceFade = 1.0 - smoothstep(LITE3D_SSS_FADE_START, LITE3D_SSS_FADE_END, length(rayPos));
    if (isNear(distanceFade, 0.0))
        return 1.0;

    float grazing = 1.0 - angular.NdotV;
    float minDepthThreshold = mix(LITE3D_SSS_MIN_DEPTH_THRESHOLD, LITE3D_SSS_MIN_DEPTH_THRESHOLD * 5.0, 
        grazing * grazing);
    
    // Compute ray step
    vec3 rayStep = rayDir * LITE3D_SSS_STEP_LENGTH;
    // Ray march towards the light
    float occlusion = 0.0;
    for (int i = 0; i < LITE3D_SSS_MAX_STEPS; i++)
    {
        // Step the ray
        rayPos += rayStep;
        vec2 rayUV = viewPositionToUV(rayPos);

        // Ensure the UV coordinates are inside the screen
        if (!isValidUV(rayUV))
            return 1.0;
        
        // Compute the difference between the ray's and the camera's depth
        float depth = worldToViewSpacePosition(texture(GBuffer, vec3(rayUV, 0)).xyz).z;
        float depthDelta = depth - rayPos.z;

        if (depthDelta > minDepthThreshold && depthDelta < LITE3D_SSS_MAX_DEPTH_THRESHOLD)
        {
            // Mark as occluded
            occlusion = fadeScreenEdge(rayUV) * distanceFade;
            break;
        }
    }

    return 1.0 - occlusion;
}
