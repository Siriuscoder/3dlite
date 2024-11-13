/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#include <lite3d/lite3d_misc.h>

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
/* GL_ANGLE_framebuffer_blit */
PFNGLBLITFRAMEBUFFERANGLEPROC glBlitFramebufferPtr = NULL;
/* GL_EXT_occlusion_query_boolean */
PFNGLGENQUERIESEXTPROC glGenQueriesPtr = NULL;
PFNGLDELETEQUERIESEXTPROC glDeleteQueriesPtr = NULL;
PFNGLISQUERYEXTPROC glIsQueryPtr = NULL;
PFNGLBEGINQUERYEXTPROC glBeginQueryPtr = NULL;
PFNGLENDQUERYEXTPROC glEndQueryPtr = NULL;
PFNGLGETQUERYIVEXTPROC glGetQueryivPtr = NULL;
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivPtr = NULL;
/* GL_OES_texture_3D */
PFNGLTEXIMAGE3DOESPROC glTexImage3DPtr = NULL;
PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DPtr = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DPtr = NULL;
PFNGLFRAMEBUFFERTEXTURE3DOESPROC glFramebufferTexture3DPtr = NULL;
/* GL_OES_geometry_shader */
PFNGLFRAMEBUFFERTEXTUREOESPROC glFramebufferTexturePtr = NULL;

#endif

int lite3d_check_vertex_array_object(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return SDL_GL_ExtensionSupported("GL_OES_vertex_array_object") == SDL_TRUE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_vertex_array_object || GLEW_APPLE_vertex_array_object || GLEW_VERSION_3_0;
#endif
}

int lite3d_check_instanced_arrays(void)
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

int lite3d_check_copy_buffer(void)
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

int lite3d_check_uniform_buffer(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_uniform_buffer_object || GLEW_VERSION_3_0;
#endif
}

int lite3d_check_ssbo(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_ARB_shader_storage_buffer_object || GLEW_VERSION_4_2;
#endif
}

int lite3d_check_texture_compression_rgtc(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_compression_rgtc") == SDL_TRUE;
#else
    return GLEW_EXT_texture_compression_rgtc || GLEW_ARB_texture_compression_rgtc || GLEW_VERSION_3_0;
#endif
}

int lite3d_check_texture_compression_s3tc(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_compression_s3tc") == SDL_TRUE &&
        SDL_GL_ExtensionSupported("GL_EXT_texture_compression_s3tc_srgb") == SDL_TRUE;
#else
    return GLEW_EXT_texture_compression_s3tc || GLEW_NV_texture_compression_s3tc;
#endif
}

int lite3d_check_texture_filter_anisotropic(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic") == SDL_TRUE;
#else
    return GLEW_EXT_texture_filter_anisotropic || GLEW_ARB_texture_filter_anisotropic;
#endif
}

int lite3d_check_map_buffer(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_mapbuffer") == SDL_TRUE;
#else
    return LITE3D_TRUE;
#endif    
}

int lite3d_check_gl_version(void)
{
#ifdef GLES
    return LITE3D_TRUE;
#else
    return GLEW_VERSION_3_1;
#endif
}

int lite3d_check_tbo(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else 
    return GLEW_VERSION_3_1;
#endif    
}

int lite3d_check_seamless_cube_map(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else 
    return GLEW_ARB_seamless_cube_map || GLEW_ARB_seamless_cubemap_per_texture;
#endif      
}

int lite3d_check_geometry_shader(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_geometry_shader") == SDL_TRUE || 
        SDL_GL_ExtensionSupported("GL_EXT_geometry_shader") == SDL_TRUE;
#else 
    return GLEW_ARB_geometry_shader4 || GLEW_VERSION_3_2;
#endif 
}

int lite3d_check_renderbuffer_storage_multisample(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return LITE3D_TRUE;
#endif 
}

int lite3d_check_texture_multisample(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_texture_multisample || GLEW_VERSION_3_2;
#endif 
}

int lite3d_check_occlusion_query(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return SDL_GL_ExtensionSupported("GL_EXT_occlusion_query_boolean") == SDL_TRUE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_occlusion_query2 || GLEW_VERSION_3_3;
#endif 
}

int lite3d_check_framebuffer_blit(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return SDL_GL_ExtensionSupported("GL_ANGLE_framebuffer_blit") == SDL_TRUE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_EXT_framebuffer_blit || GLEW_VERSION_3_0;
#endif 
}

int lite3d_check_depth32(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_depth32") == SDL_TRUE;
#else
    return GLEW_VERSION_1_4;
#endif 
}

int lite3d_check_shadow_samplers(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return SDL_GL_ExtensionSupported("GL_EXT_shadow_samplers") == SDL_TRUE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_VERSION_3_0;
#endif 
}

int lite3d_check_srgb(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_VERSION_2_1;
#endif 
}

int lite3d_check_texture3D(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_texture_3D") == SDL_TRUE;
#else
    return GLEW_VERSION_3_0;
#endif 
}

int lite3d_check_texture_swizzle(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_texture_swizzle || GLEW_EXT_texture_swizzle || GLEW_VERSION_3_3;
#endif 
}

int lite3d_check_texture_storage(void)
{
#ifdef GLES
#   ifdef WITH_GLES2
    return LITE3D_FALSE;
#   else
    return LITE3D_TRUE;
#   endif
#else
    return GLEW_ARB_texture_storage || GLEW_VERSION_4_2;
#endif 
}

int lite3d_check_texture_storage_multisample(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_ARB_texture_storage_multisample || GLEW_VERSION_4_3;
#endif 
}

int lite3d_check_texture_cube_map_array(void)
{
#ifdef GLES
    return SDL_GL_ExtensionSupported("GL_OES_texture_cube_map_array") == SDL_TRUE || 
        SDL_GL_ExtensionSupported("GL_EXT_texture_cube_map_array") == SDL_TRUE;
#else
    return GLEW_ARB_texture_cube_map_array || GLEW_VERSION_4_0;
#endif
}

int lite3d_check_debug_context(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_KHR_debug;
#endif
}

int lite3d_check_bindless_texture(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_ARB_bindless_texture;
#endif
}

int lite3d_check_shader_draw_parameters(void)
{
#ifdef GLES
    return LITE3D_FALSE;
#else
    return GLEW_ARB_shader_draw_parameters;
#endif
}

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

int lite3d_init_gl_extensions_binding(void)
{
#ifndef GLES
    GLenum err;
    if ((err = glewInit()) != GLEW_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: glewInit failed: %s", LITE3D_CURRENT_FUNCTION, glewGetErrorString(err));
        return LITE3D_FALSE;
    }
    
    return LITE3D_TRUE;
#else

    if (lite3d_check_map_buffer())
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

    if (lite3d_check_texture3D())
    {
        glTexImage3DPtr = SDL_GL_GetProcAddress("glTexImage3DOES");
        glTexSubImage3DPtr = SDL_GL_GetProcAddress("glTexSubImage3DOES");
        glCompressedTexSubImage3DPtr = SDL_GL_GetProcAddress("glCompressedTexSubImage3DOES");
        glFramebufferTexture3DPtr = SDL_GL_GetProcAddress("glFramebufferTexture3DOES");
    }
    else
    {
        glTexImage3DPtr = glTexImage3D_stub;
        glTexSubImage3DPtr = glTexSubImage3D_stub;
        glCompressedTexSubImage3DPtr = glCompressedTexSubImage3D_stub;
        glFramebufferTexture3DPtr = glFramebufferTexture3D_stub;
    }

    if (lite3d_check_geometry_shader())
    {
        glFramebufferTexturePtr = SDL_GL_GetProcAddress("glFramebufferTextureOES");
    }
    else
    {
        glFramebufferTexturePtr = glFramebufferTexture_stub;
    }
    
#ifdef WITH_GLES2
    if (lite3d_check_instanced_arrays())
    {
        glDrawArraysInstancedPtr = SDL_GL_GetProcAddress("glDrawArraysInstancedANGLE");
        glDrawElementsInstancedPtr = SDL_GL_GetProcAddress("glDrawElementsInstancedANGLE");
        glVertexAttribDivisorPtr = SDL_GL_GetProcAddress("glVertexAttribDivisorANGLE");
    }

    if (lite3d_check_vertex_array_object())
    {
        glBindVertexArrayPtr = SDL_GL_GetProcAddress("glBindVertexArrayOES");
        glDeleteVertexArraysPtr = SDL_GL_GetProcAddress("glDeleteVertexArraysOES");
        glGenVertexArraysPtr = SDL_GL_GetProcAddress("glGenVertexArraysOES");
        glIsVertexArrayPtr = SDL_GL_GetProcAddress("glIsVertexArrayOES");
    }

    if (lite3d_check_framebuffer_blit())
    {
        glBlitFramebufferPtr = SDL_GL_GetProcAddress("glBlitFramebufferANGLE");
    }
    else
    {
        glBlitFramebufferPtr = glBlitFramebuffer_stub;
    }

    if (lite3d_check_occlusion_query())
    {
        glGenQueriesPtr = SDL_GL_GetProcAddress("glGenQueriesEXT");
        glDeleteQueriesPtr = SDL_GL_GetProcAddress("glDeleteQueriesEXT");
        glIsQueryPtr = SDL_GL_GetProcAddress("glIsQueryEXT");
        glBeginQueryPtr = SDL_GL_GetProcAddress("glBeginQueryEXT");
        glEndQueryPtr = SDL_GL_GetProcAddress("glEndQueryEXT");
        glGetQueryivPtr = SDL_GL_GetProcAddress("glGetQueryivEXT");
        glGetQueryObjectuivPtr = SDL_GL_GetProcAddress("glGetQueryObjectuivEXT");
    }
    else
    {
        glGenQueriesPtr = glGenQueries_stub;
        glDeleteQueriesPtr = glDeleteQueries_stub;
        glIsQueryPtr = glIsQuery_stub;
        glBeginQueryPtr = glBeginQuery_stub;
        glEndQueryPtr = glEndQuery_stub;
        glGetQueryivPtr = glGetQueryiv_stub;
        glGetQueryObjectuivPtr = glGetQueryObjectuiv_stub;
    }

#endif
    
    return LITE3D_TRUE;
#endif
}

#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

/* stub functions */
void glTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexSubImage3D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexImage3D_stub(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage3D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glCompressedTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glCompressedTexSubImage3D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glFramebufferTexture3D_stub(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glFramebufferTexture3D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();   
}

void glTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexSubImage1D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexImage1D_stub(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage1D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glCompressedTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glCompressedTexSubImage1D is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void* glMapBuffer_stub(GLenum target, GLenum access)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glMapBuffer is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
    return NULL;
}

GLboolean glUnmapBuffer_stub(GLenum target)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glUnmapBuffer is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
    return GL_FALSE;
}

void glGetBufferPointerv_stub(GLenum target, GLenum pname, void** params)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGetBufferPointerv is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexBuffer_stub(GLenum target, GLenum internalFormat, GLuint buffer)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexBuffer is ot supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glRenderbufferStorageMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glRenderbufferStorageMultisample is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexImage2DMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage2DMultisample is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexImage3DMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glTexImage3DMultisample is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glBlitFramebuffer_stub(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glBlitFramebuffer is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glCopyBufferSubData_stub(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glCopyBufferSubData is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glGenQueries_stub(GLsizei n, GLuint *ids)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGenQueries is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glDeleteQueries_stub(GLsizei n, const GLuint *ids)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glDeleteQueries is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

GLboolean glIsQuery_stub(GLuint id)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glIsQuery is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
    return GL_FALSE;
}

void glBeginQuery_stub(GLenum target, GLuint id)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glBeginQuery is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glEndQuery_stub(GLenum target)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glEndQuery is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glGetQueryiv_stub(GLenum target, GLenum pname, GLint *params)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGetQueryiv is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glGetQueryObjectuiv_stub(GLuint id, GLenum pname, GLuint *params)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGetQueryObjectuiv is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glFramebufferTextureLayer_stub(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glFramebufferTextureLayer is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glFramebufferTexture_stub(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glFramebufferTexture is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}

void glTexStorage1D_stub(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glFramebufferTexture is not supported..", LITE3D_CURRENT_FUNCTION);
    lite3d_misc_gl_set_not_supported();
}
