SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular)
{
    SufraceLighting result;
    result.diffuseEnergy = vec3(0.0);
    result.diffuse = LITE3D_BASE_AMBIENT_LIGHT;
    result.specular = vec3(0.0);
    
    return result;
}
