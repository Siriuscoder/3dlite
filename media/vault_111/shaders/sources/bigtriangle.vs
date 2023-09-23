layout(location = 0) in vec2 vertex;

#ifdef USE_INSTANCING 
layout(location = 1) in mat4 modelMatrix;
uniform mat4 projViewMatrix;
#else
uniform mat4 screenMatrix;
#endif

out vec2 iuv;

void main()
{
    iuv = vertex;
#ifdef USE_INSTANCING 
    gl_Position = projViewMatrix * modelMatrix * vec4(vertex.xy, 0.0, 1.0);
#else
    gl_Position = screenMatrix * vec4(vertex.xy, 0.0, 1.0);
#endif
}