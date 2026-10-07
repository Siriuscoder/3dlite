uniform sampler2DArray GBuffer;

float SSS(vec3 P, vec3 L, float minDepthThreshold)
{
    // Compute ray position and direction (in view-space)
    vec3 rayPos = worldToViewSpacePosition(P);
    vec3 rayDir = worldToViewSpaceDirection(L);
    
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
            occlusion = fadeScreenEdge(rayUV);
            break;
        }
    }

    return 1.0 - occlusion;
}
