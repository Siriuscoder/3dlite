/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_log.h>
#include <SDL_assert.h>
#include <SDL_video.h>

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_glext.h>

#ifdef GLES

/* GL_OES_vertex_array_object */
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayPtr = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysPtr = NULL;
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysPtr = NULL;
PFNGLISVERTEXARRAYOESPROC glIsVertexArrayPtr = NULL;
/* GL_OES_mapbuffer */
PFNGLMAPBUFFEROESPROC glMapBufferPtr = NULL;
PFNGLUNMAPBUFFEROESPROC glUnmapBufferPtr = NULL;
PFNGLGETBUFFERPOINTERVOESPROC glGetBufferPointervPtr = NULL;
/* GL_ANGLE_instanced_arrays */
PFNGLDRAWARRAYSINSTANCEDANGLEPROC glDrawArraysInstancedPtr = NULL;
PFNGLDRAWELEMENTSINSTANCEDANGLEPROC glDrawElementsInstancedPtr = NULL;
PFNGLVERTEXATTRIBDIVISORANGLEPROC glVertexAttribDivisorPtr = NULL;

#endif

int lite3d_check_vertex_array_object()
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_vertex_array_object") == SDL_TRUE;
#else
    return GLEW_ARB_vertex_array_object;
#endif
}

int lite3d_check_instanced_arrays()
{
#ifdef GLES
#   ifdef WITH_GLES2
    return SDL_GL_ExtensionSupported("GL_ANGLE_instanced_arrays") == SDL_TRUE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_VERSION_3_3 || GLEW_ARB_instanced_arrays;
#endif
}

int lite3d_check_vertex_buffer_object()
{
#ifdef GLES
    return LITE3D_TRUE;
#else
    return GLEW_ARB_vertex_buffer_object;
#endif
}

int lite3d_check_copy_buffer()
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_copy_buffer;
#endif
}

int lite3d_check_uniform_buffer()
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_uniform_buffer_object;
#endif
}

int lite3d_check_ssbo()
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_ARB_shader_storage_buffer_object;
#endif
}

int lite3d_check_texture_compression()
{
#ifdef GLES
    return LITE3D_TRUE;
#else
    return GLEW_ARB_texture_compression;
#endif
}

int lite3d_check_texture_compression_s3tc()
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_compression_s3tc") == SDL_TRUE &&
        SDL_GL_ExtensionSupported("GL_EXT_texture_compression_rgtc") == SDL_TRUE;
#else
    return GLEW_EXT_texture_compression_s3tc && GLEW_EXT_texture_compression_rgtc;
#endif
}

int lite3d_check_texture_compression_dxt1()
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_compression_dxt1") == SDL_TRUE;
#else
    return GLEW_EXT_texture_compression_dxt1;
#endif
}

int lite3d_check_texture_filter_anisotropic()
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic") == SDL_TRUE;
#else
    return GLEW_EXT_texture_filter_anisotropic;
#endif
}

int lite3d_check_map_buffer()
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_mapbuffer") == SDL_TRUE;
#else
    return LITE3D_TRUE;
#endif    
}

int lite3d_check_gl_version()
{
#ifdef GLES
    return LITE3D_TRUE;
#else
    return GLEW_VERSION_3_1;
#endif
}

int lite3d_check_tbo()
{
#ifdef GLES
#   ifdef GL_ES_VERSION_3_2
    return LITE3D_TRUE;
#   else
    return LITE3D_FALSE;
#   endif
#else 
    return GLEW_VERSION_3_1;
#endif    
}

int lite3d_check_seamless_cube_map()
{
#ifdef GLES
    return LITE3D_FALSE;
#else 
    return GLEW_ARB_seamless_cube_map || GLEW_ARB_seamless_cubemap_per_texture;
#endif      
}

int lite3d_check_geometry_shader()
{
#ifdef GLES
    return LITE3D_FALSE;
#else 
    return GLEW_ARB_geometry_shader4 || GLEW_VERSION_3_2;
#endif 
}

int lite3d_init_gl_extensions_binding()
{
#ifndef GLES
    GLenum err;
    if((err = glewInit()) != GLEW_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: glewInit failed: %s", LITE3D_CURRENT_FUNCTION, glewGetErrorString(err));
        return LITE3D_FALSE;
    }
    
    return LITE3D_TRUE;
#else
    if(lite3d_check_vertex_array_object())
    {
        glBindVertexArrayPtr = SDL_GL_GetProcAddress("glBindVertexArrayOES");
        glDeleteVertexArraysPtr = SDL_GL_GetProcAddress("glDeleteVertexArraysOES");
        glGenVertexArraysPtr = SDL_GL_GetProcAddress("glGenVertexArraysOES");
        glIsVertexArrayPtr = SDL_GL_GetProcAddress("glIsVertexArrayOES");
    }
    
    if(lite3d_check_map_buffer())
    {
        glMapBufferPtr = SDL_GL_GetProcAddress("glMapBufferOES");
        glUnmapBufferPtr = SDL_GL_GetProcAddress("glUnmapBufferOES");
        glGetBufferPointervPtr = SDL_GL_GetProcAddress("glGetBufferPointervOES");
    }
    else
    {
        glMapBufferPtr = glMapBuffer_stub;
        glUnmapBufferPtr = glUnmapBuffer_stub;
        glGetBufferPointervPtr = glGetBufferPointerv_stub;
    }
    
#ifdef WITH_GLES2
    if(lite3d_check_instanced_arrays())
    {
        glDrawArraysInstancedPtr = SDL_GL_GetProcAddress("glDrawArraysInstancedANGLE");
        glDrawElementsInstancedPtr = SDL_GL_GetProcAddress("glDrawElementsInstancedANGLE");
        glVertexAttribDivisorPtr = SDL_GL_GetProcAddress("glVertexAttribDivisorANGLE");
    }
#endif
    
    return LITE3D_TRUE;
#endif
}

/* stub functions */
void glTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexSubImage3D not supported..", LITE3D_CURRENT_FUNCTION);    
}

void glTexImage3D_stub(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage3D not supported..", LITE3D_CURRENT_FUNCTION);       
}

void glCompressedTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glCompressedTexSubImage3D not supported..", LITE3D_CURRENT_FUNCTION);       
}

void glTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexSubImage1D not supported..", LITE3D_CURRENT_FUNCTION);       
}

void glTexImage1D_stub(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage1D not supported..", LITE3D_CURRENT_FUNCTION);       
}

void glCompressedTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glCompressedTexSubImage1D not supported..", LITE3D_CURRENT_FUNCTION);       
}

void glMapBuffer_stub(GLenum target, GLenum access)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glMapBuffer not supported..", LITE3D_CURRENT_FUNCTION);   
}

void glUnmapBuffer_stub(GLenum target)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glUnmapBuffer not supported..", LITE3D_CURRENT_FUNCTION);   
}

void glGetBufferPointerv_stub(GLenum target, GLenum pname, void** params)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGetBufferPointerv not supported..", LITE3D_CURRENT_FUNCTION);
}

void glTexBuffer_stub(GLenum target, GLenum internalFormat, GLuint buffer)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexBuffer not supported..", LITE3D_CURRENT_FUNCTION);
}