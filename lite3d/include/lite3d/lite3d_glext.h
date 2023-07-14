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
#ifndef LITE3D_GLEXT_H
#define	LITE3D_GLEXT_H

#include <lite3d/lite3d_gl.h>

int lite3d_check_vertex_array_object();
int lite3d_check_instanced_arrays();
int lite3d_check_vertex_buffer_object();
int lite3d_check_copy_buffer();
int lite3d_check_texture_compression();
int lite3d_check_texture_compression_s3tc();
int lite3d_check_texture_compression_dxt1();
int lite3d_check_texture_filter_anisotropic();
int lite3d_check_map_buffer();
int lite3d_check_gl_version();
int lite3d_init_gl_extensions_binding();
int lite3d_check_tbo();
int lite3d_check_uniform_buffer();
int lite3d_check_ssbo();
int lite3d_check_seamless_cube_map();
int lite3d_check_geometry_shader();
int lite3d_check_renderbuffer_storage_multisample();
int lite3d_check_texture_multisample();
int lite3d_check_framebuffer_blit();
int lite3d_check_occlusion_query();


/* stub functions */
void glTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
void glTexImage3D_stub(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
void glCompressedTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
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

#   ifndef GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS
#       define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT
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

#   ifdef WITH_GLES2
#       define glDrawArraysInstanced glDrawArraysInstancedPtr
#       define glDrawElementsInstanced glDrawElementsInstancedPtr
#       define glVertexAttribDivisor glVertexAttribDivisorPtr

#       define glBindVertexArray glBindVertexArrayPtr
#       define glDeleteVertexArrays glDeleteVertexArraysPtr
#       define glGenVertexArrays glGenVertexArraysPtr
#       define glIsVertexArray glIsVertexArrayPtr

#       define glBlitFramebuffer glBlitFramebufferPtr 

#       define glTexSubImage3D glTexSubImage3D_stub
#       define glTexImage3D glTexImage3D_stub
#       define glCompressedTexSubImage3D glCompressedTexSubImage3D_stub
#       define glRenderbufferStorageMultisample glRenderbufferStorageMultisample_stub
#       define glCopyBufferSubData glCopyBufferSubData_stub

#       ifndef GL_READ_FRAMEBUFFER
#           define GL_READ_FRAMEBUFFER GL_READ_FRAMEBUFFER_ANGLE
#       endif

#       ifndef GL_DRAW_FRAMEBUFFER
#           define GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER_ANGLE
#       endif

#       ifndef GL_RED
#           define GL_RED 0x1903
#       endif

#       ifndef GL_RG
#           define GL_RG 0x8227
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

#   endif

#   define glMapBuffer glMapBufferPtr
#   define glUnmapBuffer glUnmapBufferPtr
#   define glGetBufferPointerv glGetBufferPointervPtr
#   define glTexBuffer glTexBuffer_stub /* TODO GL_OES_texture_buffer */
/* Not supported at all in GLES */
#   define glTexSubImage1D glTexSubImage1D_stub
#   define glTexImage1D glTexImage1D_stub
#   define glCompressedTexSubImage1D glCompressedTexSubImage1D_stub
#   define glTexImage2DMultisample glTexImage2DMultisample_stub
#   define glTexImage3DMultisample glTexImage3DMultisample_stub
#endif

#endif