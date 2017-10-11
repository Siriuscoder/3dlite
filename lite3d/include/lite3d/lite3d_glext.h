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

/* stub functions */
void glTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
void glTexImage3D_stub(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
void glCompressedTexSubImage3D_stub(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
void glTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
void glTexImage1D_stub(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
void glCompressedTexSubImage1D_stub(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
void glMapBuffer_stub(GLenum target, GLenum access);
void glUnmapBuffer_stub(GLenum target);
void glGetBufferPointerv_stub(GLenum target, GLenum pname, void** params);
void glTexBuffer_stub(GLenum target, GLenum internalFormat, GLuint buffer);

#ifdef GLES

#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif

#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif

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

#if defined GL_ES_VERSION_3_2
#elif defined GL_ES_VERSION_3_1
#   define glTexBuffer glTexBuffer_stub
#elif defined GL_ES_VERSION_3_0
#   define glTexSubImage3D glTexSubImage3D_stub
#   define glTexImage3D glTexImage3D_stub
#   define glCompressedTexSubImage3D glCompressedTexSubImage3D_stub
#   define glTexBuffer glTexBuffer_stub
#elif defined GL_ES_VERSION_2_0

#   define glDrawArraysInstanced glDrawArraysInstancedPtr
#   define glDrawElementsInstanced glDrawElementsInstancedPtr
#   define glVertexAttribDivisor glVertexAttribDivisorPtr

#   define glTexSubImage3D glTexSubImage3D_stub
#   define glTexImage3D glTexImage3D_stub
#   define glCompressedTexSubImage3D glCompressedTexSubImage3D_stub
#   define glTexSubImage1D glTexSubImage1D_stub
#   define glTexImage1D glTexImage1D_stub
#   define glCompressedTexSubImage1D glCompressedTexSubImage1D_stub
#   define glTexBuffer glTexBuffer_stub
#endif

#   define glBindVertexArray glBindVertexArrayPtr
#   define glDeleteVertexArrays glDeleteVertexArraysPtr
#   define glGenVertexArrays glGenVertexArraysPtr
#   define glIsVertexArray glIsVertexArrayPtr
#   define glMapBuffer glMapBufferPtr
#   define glUnmapBuffer glUnmapBufferPtr
#   define glGetBufferPointerv glGetBufferPointervPtr

#endif

#endif