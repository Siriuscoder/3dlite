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
 *****************************************************************************/
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_vbo.h>

static GLenum vboModeEnum[] = {
    GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, 
    GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, 
    GL_DYNAMIC_COPY
};

static GLenum vboMapModeEnum[] = {
    GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE
};

static GLint gUBOMaxSize = -1;
static GLint gSSBOMaxSize = -1;
static GLint gTBOMaxSize = -1;

void lite3d_vbo_get_limitations(int *UBOMaxSize, int *TBOMaxSize, int *SSBOMaxSize)
{
    if (UBOMaxSize)
    {
        *UBOMaxSize = gUBOMaxSize;
    }

    if (TBOMaxSize)
    {
        *TBOMaxSize = gTBOMaxSize;
    }

    if (SSBOMaxSize)
    {
        *SSBOMaxSize = gSSBOMaxSize;
    }
}

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

static int check_buffer_access(uint16_t access)
{
    if (access >= (sizeof(vboModeEnum) / sizeof(vboModeEnum[0])))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: Invalid VBO access %d", LITE3D_CURRENT_FUNCTION, access);
        return LITE3D_FALSE;
    }

#ifdef WITH_GLES2
    switch (vboModeEnum[access])
    {
        case GL_STREAM_READ:
        case GL_STREAM_COPY:
        case GL_STATIC_READ:
        case GL_STATIC_COPY:
        case GL_DYNAMIC_READ:
        case GL_DYNAMIC_COPY:
        {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION,
                "%s: VBO access %d is not supported in GLES2", LITE3D_CURRENT_FUNCTION, access);
            return LITE3D_FALSE;
        }
    }
#endif

    return LITE3D_TRUE;
}

static int vbo_buffer_extend(uint32_t vboID, size_t expandSize, uint16_t access)
{
    int32_t originSize;
    uint32_t tmpVbo;

    if (!lite3d_check_copy_buffer())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: Fast coping buffers via glCopyBufferSubData is not supported",
            LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    if (!check_buffer_access(access))
    {
        return LITE3D_FALSE;
    }

    glBindBuffer(GL_COPY_READ_BUFFER, vboID);
    glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &originSize);

    glGenBuffers(1, &tmpVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, tmpVbo);

    /* allocate tmp buffer */
    glBufferData(GL_COPY_WRITE_BUFFER, originSize, NULL, GL_STATIC_COPY);

    /* copy data to tmp buffer */
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, originSize);

    if (LITE3D_CHECK_GL_ERROR)
    {
        glDeleteBuffers(1, &tmpVbo);
        return LITE3D_FALSE;
    }

    glBindBuffer(GL_COPY_READ_BUFFER, tmpVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, vboID);

    /* reallocate our buffer */
    glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &originSize);
    glBufferData(GL_COPY_WRITE_BUFFER, originSize + expandSize, NULL, vboModeEnum[access]);

    /* copy data back to our buffer */
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, originSize);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

    glDeleteBuffers(1, &tmpVbo);

    return LITE3D_TRUE;
}

int lite3d_vbo_technique_init(void)
{
    int var;

    if (!lite3d_check_map_buffer())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: !!! Buffer mapping not supported !!!",
            LITE3D_CURRENT_FUNCTION);
    }

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &var);

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "GL_MAX_VERTEX_ATTRIBS: %d",
        var);

#ifndef WITH_GLES2
    if (lite3d_check_uniform_buffer())
    {
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_VERTEX_UNIFORM_BLOCKS: %d",
            var);

        if (lite3d_check_geometry_shader())
        {
            glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &var);
            SDL_LogDebug(
                SDL_LOG_CATEGORY_APPLICATION,
                "GL_MAX_GEOMETRY_UNIFORM_BLOCKS: %d",
                var);

            glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &var);
            SDL_LogDebug(
                SDL_LOG_CATEGORY_APPLICATION,
                "GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS: %d",
                var);
        }

        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_FRAGMENT_UNIFORM_BLOCKS: %d",
            var);

        glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_COMBINED_UNIFORM_BLOCKS: %d",
            var);

        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_UNIFORM_BUFFER_BINDINGS: %d",
            var);

        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gUBOMaxSize);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_UNIFORM_BLOCK_SIZE: %d",
            gUBOMaxSize);

        glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS: %d",
            var);

        glGetIntegerv(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS: %d",
            var);
    }
#endif

#ifndef GLES
    if (lite3d_check_ssbo())
    {
        glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS: %d",
             var);

        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &var);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: %d",
             var);

        glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &gSSBOMaxSize);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_SHADER_STORAGE_BLOCK_SIZE: %d",
             gSSBOMaxSize);
    }

    if (lite3d_check_tbo())
    {
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &gTBOMaxSize);
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "GL_MAX_TEXTURE_BUFFER_SIZE: %d",
             gTBOMaxSize);
    }

#endif

    return LITE3D_TRUE;
}

int lite3d_vbo_init(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);

    memset(vbo, 0, sizeof (lite3d_vbo));

    lite3d_misc_gl_error_stack_clean();

    /* gen buffer for store data */
    glGenBuffers(1, &vbo->vboID);

    if (!LITE3D_CHECK_GL_ERROR)
    {
        vbo->role = GL_ARRAY_BUFFER;
        lite3d_render_stats_get()->vboCount++;
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

int lite3d_ibo_init(struct lite3d_vbo *vbo)
{
    if (!lite3d_vbo_init(vbo))
    {
        return LITE3D_FALSE;
    }

    vbo->role = GL_ELEMENT_ARRAY_BUFFER;
    lite3d_render_stats_get()->iboCount++;
    return LITE3D_TRUE;
}

int lite3d_ssbo_init(struct lite3d_vbo *vbo)
{
    if (!lite3d_check_ssbo())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SSBO is not supported",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    if (!lite3d_vbo_init(vbo))
    {
        return LITE3D_FALSE;
    }

    vbo->role = GL_SHADER_STORAGE_BUFFER;
    lite3d_render_stats_get()->ssboCount++;
    return LITE3D_TRUE;
}

int lite3d_ubo_init(struct lite3d_vbo *vbo)
{
    if (!lite3d_check_uniform_buffer())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Uniform buffers is not supported",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    if (!lite3d_vbo_init(vbo))
    {
        return LITE3D_FALSE;
    }

    vbo->role = GL_UNIFORM_BUFFER;
    lite3d_render_stats_get()->uboCount++;
    return LITE3D_TRUE;
}

void lite3d_vbo_purge(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);

    if (vbo->vboID > 0)
    {
        lite3d_render_stats *s = lite3d_render_stats_get();
        glDeleteBuffers(1, &vbo->vboID);
        
        s->vboCount--;
        if (vbo->role == GL_ELEMENT_ARRAY_BUFFER)
            s->iboCount--;
        if (vbo->role == GL_SHADER_STORAGE_BUFFER)
            s->ssboCount--;
        if (vbo->role == GL_UNIFORM_BUFFER)
            s->uboCount--;
    }

    memset(vbo, 0, sizeof(lite3d_vbo));
}

int lite3d_vbo_extend(struct lite3d_vbo *vbo, size_t addSize, uint16_t access)
{
    SDL_assert(vbo);

    lite3d_misc_gl_error_stack_clean();

    if (vbo->role == GL_UNIFORM_BUFFER && gUBOMaxSize > 0 && vbo->size + addSize > gUBOMaxSize)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: UBO is too large, limit is %d bytes, requested %zu bytes", LITE3D_CURRENT_FUNCTION,
            gUBOMaxSize, vbo->size + addSize);
        return LITE3D_FALSE;
    }

    vbo->access = access;
    if (vbo->size > 0)
    {
        if (!vbo_buffer_extend(vbo->vboID, addSize, access))
        {
            return LITE3D_FALSE;
        }

        vbo->size += addSize;
    }
    else
    {
        // relocate not needed, overwise may cause crash on some hardware
        if (!lite3d_vbo_buffer(vbo, NULL, addSize, access))
        {
            return LITE3D_FALSE;
        }
    }

    return LITE3D_TRUE;
}

void *lite3d_vbo_map(struct lite3d_vbo *vbo, uint16_t access)
{
    void *mapped;

    if (!lite3d_check_map_buffer())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Mapping buffers to host memory is not supported",
            LITE3D_CURRENT_FUNCTION);

        return NULL;
    }

    if (access >= (sizeof(vboMapModeEnum) / sizeof(vboMapModeEnum[0])))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: Invalid VBO Map access %d", LITE3D_CURRENT_FUNCTION, access);
        return LITE3D_FALSE;
    }

#ifdef GLES
    switch (vboMapModeEnum[access])
    {
        case GL_READ_ONLY:
        case GL_READ_WRITE:
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s: GLES MapBuffer supports only write operations (GL_WRITE_ONLY)",
                LITE3D_CURRENT_FUNCTION);
            return NULL;
        }
    }
#endif

    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    glBindBuffer(vbo->role, vbo->vboID);
    mapped = glMapBuffer(vbo->role, vboMapModeEnum[access]);
    if (LITE3D_CHECK_GL_ERROR)
    {
        glBindBuffer(vbo->role, 0);
        return NULL;
    }

    glBindBuffer(vbo->role, 0);
    return mapped;
}

void lite3d_vbo_unmap(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);
    glBindBuffer(vbo->role, vbo->vboID);
    glUnmapBuffer(vbo->role);
    glBindBuffer(vbo->role, 0);
}

int lite3d_vbo_buffer(struct lite3d_vbo *vbo,
    const void *buffer, size_t size, uint16_t access)
{
    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    if (!check_buffer_access(access))
    {
        return LITE3D_FALSE;
    }

    if (vbo->role == GL_UNIFORM_BUFFER && gUBOMaxSize > 0 && size > gUBOMaxSize)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: UBO is too large, limit is %d bytes, requested %zu bytes", LITE3D_CURRENT_FUNCTION,
            gUBOMaxSize, size);
        return LITE3D_FALSE;
    }

    glBindBuffer(vbo->role, vbo->vboID);
    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    /* store data to GPU memory */
    vbo->access = access;

    glBufferData(vbo->role, size, buffer, vboModeEnum[access]);
    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    vbo->size = size;
    glBindBuffer(vbo->role, 0);

    return LITE3D_TRUE;
}

int lite3d_vbo_subbuffer(struct lite3d_vbo *vbo,
    const void *buffer, size_t offset, size_t size)
{
    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    /* copy vertices to the end of the vertex buffer */
    glBindBuffer(vbo->role, vbo->vboID);
    glBufferSubData(vbo->role, offset, size, buffer);
    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    glBindBuffer(vbo->role, 0);
    return LITE3D_TRUE;
}

int lite3d_vbo_get_buffer(const struct lite3d_vbo *vbo,
    void *buffer, size_t offset, size_t size)
{
#ifndef GLES
    SDL_assert(vbo);
    lite3d_misc_gl_error_stack_clean();

    /* copy vertices to the end of the vertex buffer */
    glBindBuffer(vbo->role, vbo->vboID);
    glGetBufferSubData(vbo->role, offset, size, buffer);
    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    glBindBuffer(vbo->role, 0);
    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: glGetBufferSubData is not supported is GLES",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}
