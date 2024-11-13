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
#ifndef LITE3D_GLEXT_H
#define	LITE3D_GLEXT_H

#include <lite3d/lite3d_gl.h>

int lite3d_check_vertex_array_object(void);
int lite3d_check_instanced_arrays(void);
int lite3d_check_vertex_buffer_object(void);
int lite3d_check_copy_buffer(void);
int lite3d_check_texture_compression_rgtc(void);
int lite3d_check_texture_compression_s3tc(void);
int lite3d_check_texture_filter_anisotropic(void);
int lite3d_check_map_buffer(void);
int lite3d_check_gl_version(void);
int lite3d_init_gl_extensions_binding(void);
int lite3d_check_tbo(void);
int lite3d_check_uniform_buffer(void);
int lite3d_check_ssbo(void);
int lite3d_check_seamless_cube_map(void);
int lite3d_check_geometry_shader(void);
int lite3d_check_renderbuffer_storage_multisample(void);
int lite3d_check_texture_multisample(void);
int lite3d_check_framebuffer_blit(void);
int lite3d_check_occlusion_query(void);
int lite3d_check_depth32(void);
int lite3d_check_shadow_samplers(void);
int lite3d_check_srgb(void);
int lite3d_check_texture3D(void);
int lite3d_check_texture_swizzle(void);
int lite3d_check_texture_storage(void);
int lite3d_check_texture_storage_multisample(void);
int lite3d_check_texture_cube_map_array(void);
int lite3d_check_debug_context(void);

/* stub functions */
void glTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
void glTexImage3D_stub(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
void glCompressedTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
void glFramebufferTexture3D_stub(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
void glTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
void glTexImage1D_stub(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
void glCompressedTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
void* glMapBuffer_stub(GLenum target, GLenum access);
GLboolean glUnmapBuffer_stub(GLenum target);
void glGetBufferPointerv_stub(GLenum target, GLenum pname, void** params);
void glTexBuffer_stub(GLenum target, GLenum internalFormat, GLuint buffer);
void glRenderbufferStorageMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
void glTexImage2DMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
void glTexImage3DMultisample_stub(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
void glBlitFramebuffer_stub(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void glCopyBufferSubData_stub(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
void glGenQueries_stub(GLsizei n, GLuint *ids);
void glDeleteQueries_stub(GLsizei n, const GLuint *ids);
GLboolean glIsQuery_stub(GLuint id);
void glBeginQuery_stub(GLenum target, GLuint id);
void glEndQuery_stub(GLenum target);
void glGetQueryiv_stub(GLenum target, GLenum pname, GLint *params);
void glGetQueryObjectuiv_stub(GLuint id, GLenum pname, GLuint *params);
void glFramebufferTextureLayer_stub(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
void glFramebufferTexture_stub(GLenum target, GLenum attachment, GLuint texture, GLint level);
void glTexStorage1D_stub(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);

#ifdef GLES

#   ifndef GL_STACK_OVERFLOW
#       define GL_STACK_OVERFLOW 0x0503
#   endif

#   ifndef GL_STACK_UNDERFLOW
#       define GL_STACK_UNDERFLOW 0x0504
#   endif

#   ifndef GL_TEXTURE_1D
#       define GL_TEXTURE_1D 0x0DE0
#   endif

#   ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#       define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#   endif

#   ifndef GL_TEXTURE_BUFFER
#       define GL_TEXTURE_BUFFER GL_TEXTURE_BUFFER_OES
#   endif

#   ifndef GL_SHADER_STORAGE_BUFFER
#       define GL_SHADER_STORAGE_BUFFER 0x90D2
#   endif

#   ifndef GL_WRITE_ONLY
#       define  GL_WRITE_ONLY GL_WRITE_ONLY_OES
#   endif

#   ifndef GL_READ_ONLY
#       define  GL_READ_ONLY 0x88B8
#   endif

#   ifndef GL_READ_WRITE
#       define  GL_READ_WRITE 0x88BA
#   endif

#   ifndef GL_GEOMETRY_SHADER
#       define GL_GEOMETRY_SHADER GL_GEOMETRY_SHADER_EXT
#   endif

#   ifndef GL_MAX_GEOMETRY_UNIFORM_BLOCKS
#       define GL_MAX_GEOMETRY_UNIFORM_BLOCKS GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT
#   endif

#   ifndef GL_MAX_GEOMETRY_OUTPUT_VERTICES
#       define GL_MAX_GEOMETRY_OUTPUT_VERTICES GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT
#   endif

#   ifndef GL_MAX_GEOMETRY_OUTPUT_COMPONENTS
#       define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT
#   endif

#   ifndef GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS
#       define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT
#   endif

#   ifndef GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS
#       define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT
#   endif

#   ifndef GL_DEPTH_COMPONENT32
#       define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#   endif

#   ifndef GL_SRGB_ALPHA
#       define GL_SRGB_ALPHA GL_SRGB_ALPHA_EXT
#   endif

#   ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#       define GL_TEXTURE_CUBE_MAP_ARRAY GL_TEXTURE_CUBE_MAP_ARRAY_OES
#   endif

/* GL_OES_vertex_array_object */    
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayPtr;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysPtr;
extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysPtr;
extern PFNGLISVERTEXARRAYOESPROC glIsVertexArrayPtr;
/* GL_OES_mapbuffer */
extern PFNGLMAPBUFFEROESPROC glMapBufferPtr;
extern PFNGLUNMAPBUFFEROESPROC glUnmapBufferPtr;
extern PFNGLGETBUFFERPOINTERVOESPROC glGetBufferPointervPtr;
/* GL_ANGLE_instanced_arrays */
extern PFNGLDRAWARRAYSINSTANCEDANGLEPROC glDrawArraysInstancedPtr;
extern PFNGLDRAWELEMENTSINSTANCEDANGLEPROC glDrawElementsInstancedPtr;
extern PFNGLVERTEXATTRIBDIVISORANGLEPROC glVertexAttribDivisorPtr;
/* GL_ANGLE_framebuffer_blit */
extern PFNGLBLITFRAMEBUFFERANGLEPROC glBlitFramebufferPtr;
/* GL_EXT_occlusion_query_boolean */
extern PFNGLGENQUERIESEXTPROC glGenQueriesPtr;
extern PFNGLDELETEQUERIESEXTPROC glDeleteQueriesPtr;
extern PFNGLISQUERYEXTPROC glIsQueryPtr;
extern PFNGLBEGINQUERYEXTPROC glBeginQueryPtr;
extern PFNGLENDQUERYEXTPROC glEndQueryPtr;
extern PFNGLGETQUERYIVEXTPROC glGetQueryivPtr;
extern PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivPtr;
extern PFNGLTEXIMAGE3DOESPROC glTexImage3DPtr;
extern PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DPtr;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DPtr;
extern PFNGLFRAMEBUFFERTEXTURE3DOESPROC glFramebufferTexture3DPtr;
/* GL_OES_geometry_shader */
extern PFNGLFRAMEBUFFERTEXTUREOESPROC glFramebufferTexturePtr;

#   ifdef WITH_GLES2
#       define glDrawArraysInstanced glDrawArraysInstancedPtr
#       define glDrawElementsInstanced glDrawElementsInstancedPtr
#       define glVertexAttribDivisor glVertexAttribDivisorPtr

#       define glBindVertexArray glBindVertexArrayPtr
#       define glDeleteVertexArrays glDeleteVertexArraysPtr
#       define glGenVertexArrays glGenVertexArraysPtr
#       define glIsVertexArray glIsVertexArrayPtr

#       define glBlitFramebuffer glBlitFramebufferPtr 

#       define glTexSubImage3D glTexSubImage3DPtr
#       define glTexImage3D glTexImage3DPtr
#       define glCompressedTexSubImage3D glCompressedTexSubImage3DPtr
#       define glRenderbufferStorageMultisample glRenderbufferStorageMultisample_stub
#       define glCopyBufferSubData glCopyBufferSubData_stub
#       define glFramebufferTextureLayer glFramebufferTextureLayer_stub

#       define glGenQueries glGenQueriesPtr
#       define glDeleteQueries glDeleteQueriesPtr
#       define glIsQuery glIsQueryPtr
#       define glBeginQuery glBeginQueryPtr
#       define glEndQuery glEndQueryPtr
#       define glGetQueryiv glGetQueryivPtr
#       define glGetQueryObjectuiv glGetQueryObjectuivPtr

#       ifndef GL_READ_FRAMEBUFFER
#           define GL_READ_FRAMEBUFFER GL_READ_FRAMEBUFFER_ANGLE
#       endif

#       ifndef GL_DRAW_FRAMEBUFFER
#           define GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER_ANGLE
#       endif

#       ifndef GL_RED
#           define GL_RED 0x1903
#       endif

#       ifndef GL_R8
#           define GL_R8 GL_RED
#       endif

#       ifndef GL_RG
#           define GL_RG 0x8227
#       endif

#       ifndef GL_RG8
#           define GL_RG8 GL_RG
#       endif

#       ifndef GL_RGB8
#           define GL_RGB8 GL_RGB
#       endif

#       ifndef GL_RGBA8
#           define GL_RGBA8 GL_RGBA
#       endif

#       ifndef GL_TEXTURE_WRAP_R
#           define GL_TEXTURE_WRAP_R GL_TEXTURE_WRAP_R_OES
#       endif

#       ifndef GL_UNIFORM_BUFFER
#           define GL_UNIFORM_BUFFER 0x8A11
#       endif

#       ifndef GL_COPY_READ_BUFFER
#           define GL_COPY_READ_BUFFER GL_COPY_READ_BUFFER_NV
#       endif

#       ifndef GL_COPY_WRITE_BUFFER
#           define GL_COPY_WRITE_BUFFER GL_COPY_WRITE_BUFFER_NV
#       endif

#       ifndef GL_STREAM_READ
#           define GL_STREAM_READ 0x88E1
#       endif

#       ifndef GL_STREAM_COPY
#           define GL_STREAM_COPY 0x88E2
#       endif

#       ifndef GL_STATIC_READ
#           define GL_STATIC_READ 0x88E5
#       endif

#       ifndef GL_STATIC_COPY
#           define GL_STATIC_COPY 0x88E6
#       endif

#       ifndef GL_DYNAMIC_READ
#           define GL_DYNAMIC_READ 0x88E9
#       endif

#       ifndef GL_DYNAMIC_COPY
#           define GL_DYNAMIC_COPY 0x88EA
#       endif

#       ifndef GL_ANY_SAMPLES_PASSED
#           define GL_ANY_SAMPLES_PASSED GL_ANY_SAMPLES_PASSED_EXT
#       endif

#       ifndef GL_ANY_SAMPLES_PASSED_CONSERVATIVE
#           define GL_ANY_SAMPLES_PASSED_CONSERVATIVE GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT
#       endif

#       ifndef GL_CURRENT_QUERY
#           define GL_CURRENT_QUERY GL_CURRENT_QUERY_EXT
#       endif

#       ifndef GL_QUERY_RESULT
#           define GL_QUERY_RESULT GL_QUERY_RESULT_EXT
#       endif

#       ifndef GL_QUERY_RESULT_AVAILABLE
#           define GL_QUERY_RESULT_AVAILABLE GL_QUERY_RESULT_AVAILABLE_EXT
#       endif

#       ifndef GL_TEXTURE_COMPARE_MODE
#           define GL_TEXTURE_COMPARE_MODE GL_TEXTURE_COMPARE_MODE_EXT
#       endif

#       ifndef GL_COMPARE_REF_TO_TEXTURE
#           define GL_COMPARE_REF_TO_TEXTURE GL_COMPARE_REF_TO_TEXTURE_EXT
#       endif

#       ifndef GL_TEXTURE_COMPARE_FUNC
#           define GL_TEXTURE_COMPARE_FUNC GL_TEXTURE_COMPARE_FUNC_EXT
#       endif

#       ifndef GL_SRGB
#           define GL_SRGB GL_SRGB_EXT
#       endif

#       ifndef GL_TEXTURE_3D
#           define GL_TEXTURE_3D GL_TEXTURE_3D_OES
#       endif

#       ifndef GL_MAX_3D_TEXTURE_SIZE
#           define GL_MAX_3D_TEXTURE_SIZE GL_MAX_3D_TEXTURE_SIZE_OES
#       endif

#       ifndef GL_DEPTH_COMPONENT24
#           define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT
#       endif

#   endif

#   define glMapBuffer glMapBufferPtr
#   define glUnmapBuffer glUnmapBufferPtr
#   define glGetBufferPointerv glGetBufferPointervPtr
#   define glFramebufferTexture3D glFramebufferTexture3DPtr
#   define glFramebufferTexture glFramebufferTexturePtr
#   define glTexBuffer glTexBuffer_stub /* TODO GL_OES_texture_buffer */
/* Not supported at all in GLES */
#   define glTexSubImage1D glTexSubImage1D_stub
#   define glTexImage1D glTexImage1D_stub
#   define glCompressedTexSubImage1D glCompressedTexSubImage1D_stub
#   define glTexImage2DMultisample glTexImage2DMultisample_stub
#   define glTexImage3DMultisample glTexImage3DMultisample_stub
#   define glTexStorage1D glTexStorage1D_stub
#endif

#endif
