/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_vbo.h>

static int gMaxVertexAttribs;
static int gMapBufferSupport = 0;

/*
Name

    ARB_vertex_buffer_object

Name Strings

    GL_ARB_vertex_buffer_object
    GLX_ARB_vertex_buffer_object

Overview

    This extension defines an interface that allows various types of data
    (especially vertex array data) to be cached in high-performance
    graphics memory on the server, thereby increasing the rate of data
    transfers.

    Chunks of data are encapsulated within "buffer objects", which
    conceptually are nothing more than arrays of bytes, just like any
    chunk of memory.  An API is provided whereby applications can read
    from or write to buffers, either via the GL itself (glBufferData,
    glBufferSubData, glGetBufferSubData) or via a pointer to the memory.

    The latter technique is known as "mapping" a buffer.  When an
    application maps a buffer, it is given a pointer to the memory.  When
    the application finishes reading from or writing to the memory, it is
    required to "unmap" the buffer before it is once again permitted to
    use that buffer as a GL data source or sink.  Mapping often allows
    applications to eliminate an extra data copy otherwise required to
    access the buffer, thereby enhancing performance.  In addition,
    requiring that applications unmap the buffer to use it as a data
    source or sink ensures that certain classes of latent synchronization
    bugs cannot occur.

    Although this extension only defines hooks for buffer objects to be
    used with OpenGL's vertex array APIs, the API defined in this
    extension permits buffer objects to be used as either data sources or
    sinks for any GL command that takes a pointer as an argument.
    Normally, in the absence of this extension, a pointer passed into the
    GL is simply a pointer to the user's data.  This extension defines
    a mechanism whereby this pointer is used not as a pointer to the data
    itself, but as an offset into a currently bound buffer object.  The
    buffer object ID zero is reserved, and when buffer object zero is
    bound to a given target, the commands affected by that buffer binding
    behave normally.  When a nonzero buffer ID is bound, then the pointer
    represents an offset.

    In the case of vertex arrays, this extension defines not merely one
    binding for all attributes, but a separate binding for each
    individual attribute.  As a result, applications can source their
    attributes from multiple buffers.  An application might, for example,
    have a model with constant texture coordinates and variable geometry.
    The texture coordinates might be retrieved from a buffer object with
    the usage mode "STATIC_DRAW", indicating to the GL that the
    application does not expect to update the contents of the buffer
    frequently or even at all, while the vertices might be retrieved from
    a buffer object with the usage mode "STREAM_DRAW", indicating that
    the vertices will be updated on a regular basis.

    In addition, a binding is defined by which applications can source
    index data (as used by DrawElements, DrawRangeElements, and
    MultiDrawElements) from a buffer object.  On some platforms, this
    enables very large models to be rendered with no more than a few
    small commands to the graphics device.

    It is expected that a future extension will allow sourcing pixel data
    from and writing pixel data to a buffer object.
 */

static int vbo_buffer_extend(uint32_t vboID, size_t expandSize, uint16_t access)
{
#ifndef WITH_GLES2
    int32_t originSize;
    uint32_t tmpVbo;

    glBindBuffer(GL_COPY_READ_BUFFER, vboID);
    glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &originSize);

    glGenBuffers(1, &tmpVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, tmpVbo);
    /* allocate tmp buffer */
    glBufferData(GL_COPY_WRITE_BUFFER, originSize, NULL, GL_STATIC_COPY);
    /* copy data to tmp buffer */
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, originSize);

    if (lite3d_misc_check_gl_error())
    {
        glDeleteBuffers(1, &tmpVbo);
        return LITE3D_FALSE;
    }

    glBindBuffer(GL_COPY_READ_BUFFER, tmpVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, vboID);
    /* reallocate our buffer */
    glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &originSize);
    glBufferData(GL_COPY_WRITE_BUFFER, originSize + expandSize, NULL, access);
    /* copy data back to our buffer */
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, originSize);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    /* wait until async operation will be completed */
    glFinish();
    glDeleteBuffers(1, &tmpVbo);

    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: ARB_copy_buffer not supported with GLES2..", LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_vbo_technique_init(void)
{
    if (!lite3d_check_vertex_buffer_object())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL_ARB_vertex_buffer_object not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }
    
    if (!(gMapBufferSupport = lite3d_check_map_buffer()))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Buffer mapping not supported..", LITE3D_CURRENT_FUNCTION);
    }
    
#ifndef WITH_GLES2
    if (!lite3d_check_copy_buffer())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLEW_ARB_copy_buffer not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }
#endif
    
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gMaxVertexAttribs);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Max vertex attributes: %d",
        gMaxVertexAttribs);
    return LITE3D_TRUE;
}

int lite3d_vbo_init(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);

    memset(vbo, 0, sizeof (lite3d_vbo));

    lite3d_misc_gl_error_stack_clean();
    /* gen buffer for store data */
    glGenBuffers(1, &vbo->vboID);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

void lite3d_vbo_purge(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);

    glDeleteBuffers(1, &vbo->vboID);

    vbo->vboID = 0;
    vbo->size = 0;
}

int lite3d_vbo_extend(struct lite3d_vbo *vbo, size_t addSize, uint16_t access)
{
    SDL_assert(vbo);

    lite3d_misc_gl_error_stack_clean();

    vbo->access = access;
    if (!vbo_buffer_extend(vbo->vboID, addSize, access))
        return LITE3D_FALSE;

    vbo->size += addSize;

    return LITE3D_TRUE;
}

void *lite3d_vbo_map(struct lite3d_vbo *vbo, uint16_t access)
{
    void *mapped;

    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboID);
    mapped = glMapBuffer(GL_ARRAY_BUFFER, access);
    if (lite3d_misc_check_gl_error())
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return NULL;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return mapped;
}

void lite3d_vbo_unmap(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboID);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int lite3d_vbo_buffer(struct lite3d_vbo *vbo,
    const void *buffer, size_t size, uint16_t access)
{
    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboID);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* store data to GPU memory */
    vbo->access = access;
    glBufferData(GL_ARRAY_BUFFER, size, buffer, access);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    vbo->size = size;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return LITE3D_TRUE;
}

int lite3d_vbo_subbuffer(struct lite3d_vbo *vbo,
    const void *buffer, size_t offset, size_t size)
{
    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    /* copy vertices to the end of the vertex buffer */
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboID);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
    if (lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return LITE3D_TRUE;
}
