#ifndef LITE3D_BASE_AMBIENT_LIGHT
#define LITE3D_BASE_AMBIENT_LIGHT vec3(0.0)
#endif

SufraceLighting EvaluateIBL(in Surface surface, in AngularInfo angular)
{
    SufraceLighting result;
    result.diffuse = LITE3D_BASE_AMBIENT_LIGHT;
    result.specular = vec3(0.0);
    
    return result;
}
