/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <lite3d/lite3d_kazmath.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_vao.h>

static int instancingSupport = LITE3D_FALSE;
static lite3d_vao *bindedChunk = NULL;

int lite3d_vao_support_instancing(void)
{
    return instancingSupport;
}


/*
Name

    ARB_vertex_array_object

Name Strings

    GL_ARB_vertex_array_object

Overview

    This extension introduces named vertex array objects which encapsulate
    vertex array state on the client side.  These objects allow applications
    to rapidly switch between large sets of array state.  In addition, layered
    libraries can return to the default array state by simply creating and
    binding a new vertex array object.

    This extension differs from GL_APPLE_vertex_array_object in that client
    memory cannot be accessed through a non-zero vertex array object.  It also
    differs in that vertex array objects are explicitly not sharable between
    contexts.
 */

int lite3d_vao_technique_init(void)
{
    instancingSupport = LITE3D_TRUE;

    if (!lite3d_check_vertex_array_object())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL_ARB_vertex_array_object not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    if (!lite3d_check_instanced_arrays())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL_ARB_instanced_arrays not supported..", LITE3D_CURRENT_FUNCTION);
        instancingSupport = LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_vao_draw_indexed(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    /*
     * glDrawElements specifies multiple geometric primitives with very few 
     * subroutine calls. Instead of calling a GL function to pass each individual 
     * vertex, normal, texture coordinate, edge flag, or color, you can prespecify 
     * separate arrays of vertices, normals, and so on, and use them to construct a 
     * sequence of primitives with a single call to glDrawElements.
     * 
     * When glDrawElements is called, it uses count sequential elements from an 
     * enabled array, starting at indices to construct a sequence of geometric 
     * primitives. mode specifies what kind of primitives are constructed and how 
     * the array elements construct these primitives. If more than one array is 
     * enabled, each is used. If GL_VERTEX_ARRAY is not enabled, no geometric 
     * primitives are constructed.
     * Vertex attributes that are modified by glDrawElements have an unspecified 
     * value after glDrawElements returns. For example, if GL_COLOR_ARRAY is enabled, 
     * the value of the current color is undefined after glDrawElements executes. 
     * Attributes that aren't modified maintain their previous values.
     */

    glDrawElements(GL_TRIANGLES, vao->indexesCount, GL_UNSIGNED_INT, LITE3D_BUFFER_OFFSET(vao->indexesOffset));
}

void lite3d_vao_draw_indexed_instanced(struct lite3d_vao *vao, uint32_t count)
{
    /* glDrawElementsInstanced behaves identically to glDrawElements 
     * except that primcount instances of the set of elements are executed. 
     * Those attributes that have divisor N where N is other than zero 
     * (as specified by glVertexAttribDivisor) advance once every N instances. 
     */

    SDL_assert(instancingSupport);
    SDL_assert(vao);

    glDrawElementsInstanced(GL_TRIANGLES, vao->indexesCount,
        GL_UNSIGNED_INT, LITE3D_BUFFER_OFFSET(vao->indexesOffset), count);
}

void lite3d_vao_multidraw_indexed(size_t offset, size_t count)
{
    /* 
        glMultiDrawElementsIndirect specifies multiple indexed geometric primitives with very few subroutine calls. 
        glMultiDrawElementsIndirect behaves similarly to a multitude of calls to glDrawElementsInstancedBaseVertexBaseInstance,
        execpt that the parameters to glDrawElementsInstancedBaseVertexBaseInstance are stored in an array in memory 
        at the address given by indirect, separated by the stride, in basic machine units, specified by stride. 
        If stride is zero, then the array is assumed to be tightly packed in memory.

        The parameters addressed by indirect are packed into a structure that takes the form (in C):

        typedef  struct {
            uint  count;
            uint  instanceCount;
            uint  firstIndex;
            int  baseVertex;
            uint  baseInstance;
        } DrawElementsIndirectCommand;

        A single call to glMultiDrawElementsIndirect is equivalent, assuming no errors are generated to:

        GLsizei n;
        for (n = 0; n < drawcount; n++) {
            const DrawElementsIndirectCommand *cmd;
            if (stride != 0) {
                cmd = (const DrawElementsIndirectCommand  *)((uintptr)indirect + n * stride);
            } else {
                cmd = (const DrawElementsIndirectCommand  *)indirect + n;
            }

            glDrawElementsInstancedBaseVertexBaseInstance(mode,
                                                          cmd->count,
                                                          type,
                                                          cmd->firstIndex * size-of-type,
                                                          cmd->instanceCount,
                                                          cmd->baseVertex,
                                                          cmd->baseInstance);
        }

        If a buffer is bound to the GL_DRAW_INDIRECT_BUFFER binding at the time of a call to glDrawElementsIndirect, 
        indirect is interpreted as an offset, in basic machine units, into that buffer and the parameter data is read from 
        the buffer rather than from client memory.Note that indices stored in client memory are not supported. 
        If no buffer is bound to the GL_ELEMENT_ARRAY_BUFFER binding, an error will be generated. The results of the 
        operation are undefined if the reservedMustBeZero member of the parameter structure is non-zero. 
        However, no error is generated in this case.

        Vertex attributes that are modified by glDrawElementsIndirect have an unspecified value after 
        glDrawElementsIndirect returns. Attributes that aren't modified remain well defined.
    */
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, LITE3D_BUFFER_OFFSET(offset), (GLsizei)count, 0);
}

void lite3d_vao_draw(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    glDrawArrays(GL_TRIANGLES, 0, vao->verticesCount);
}

void lite3d_vao_draw_instanced(struct lite3d_vao *vao, uint32_t count)
{
    SDL_assert(instancingSupport);
    SDL_assert(vao);

    glDrawArraysInstanced(GL_TRIANGLES, 0, vao->verticesCount, count);
}

void lite3d_vao_multidraw(size_t offset, size_t count)
{
    /*
        glDrawArraysIndirect specifies multiple geometric primitives with very few subroutine calls. 
        glDrawArraysIndirect behaves similarly to glDrawArraysInstancedBaseInstance, execept that the parameters 
        to glDrawArraysInstancedBaseInstance are stored in memory at the address given by indirect.

        The parameters addressed by indirect are packed into a structure that takes the form (in C):

        typedef  struct {
            uint  count;
            uint  instanceCount;
            uint  first;
            uint  baseInstance;
        } DrawArraysIndirectCommand;

        const DrawArraysIndirectCommand *cmd = (const DrawArraysIndirectCommand *)indirect;
        glDrawArraysInstancedBaseInstance(mode, cmd->first, cmd->count, cmd->instanceCount, cmd->baseInstance);

        If a buffer is bound to the GL_DRAW_INDIRECT_BUFFER binding at the time of a call to glDrawArraysIndirect, 
        indirect is interpreted as an offset, in basic machine units, into that buffer and the parameter data is read 
        from the buffer rather than from client memory.
        In contrast to glDrawArraysInstancedBaseInstance, the first member of the parameter structure is unsigned, 
        and out-of-range indices do not generate an error.

        Vertex attributes that are modified by glDrawArraysIndirect have an unspecified value after glDrawArraysIndirect
         returns. Attributes that aren't modified remain well defined.
    */
    glMultiDrawArraysIndirect(GL_TRIANGLES, LITE3D_BUFFER_OFFSET(offset), (GLsizei)count, 0);
}

void lite3d_vao_bind(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    /* bind current vao */
    if (bindedChunk != vao)
    {
        glBindVertexArray(vao->vaoID);
        bindedChunk = vao;
    }
}

void lite3d_vao_unbind(void)
{
    /* zero bind */
    glBindVertexArray(0);
    bindedChunk = NULL;
}

int lite3d_vao_init(struct lite3d_vao *vao)
{
    SDL_assert(vao);

    memset(vao, 0, sizeof (lite3d_vao));

    lite3d_misc_gl_error_stack_clean();
    glGenVertexArrays(1, &vao->vaoID);

    if (!LITE3D_CHECK_GL_ERROR)
    {
        lite3d_render_stats_get()->vaoCount++;
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

void lite3d_vao_purge(struct lite3d_vao *vao)
{
    SDL_assert(vao);

    glDeleteVertexArrays(1, &vao->vaoID);
    lite3d_render_stats_get()->vaoCount--;
    vao->vaoID = 0;
}

int lite3d_vao_init_layout(struct lite3d_vbo *vertexBuffer,
    struct lite3d_vbo *indexBuffer,
    struct lite3d_vbo *auxBuffer,
    struct lite3d_vao *vao, 
    const struct lite3d_vao_layout *layout, 
    uint32_t layoutCount, 
    uint32_t stride, 
    uint32_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    uint32_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset)
{
    uint32_t attribIndex = 0, i = 0;
    size_t vOffset = verticesOffset;

    SDL_assert(vertexBuffer);

    /* VAO set current */
    lite3d_vao_bind(vao);
    /* use single VBO to store all data */
    lite3d_vbo_bind(vertexBuffer);
    /* bind all arrays and attribs into the current VAO */
    for (; i < layoutCount; ++i)
    {
        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex++, layout[i].count, GL_FLOAT,
            GL_FALSE, stride, LITE3D_BUFFER_OFFSET(vOffset));

        vOffset += layout[i].count * sizeof (GLfloat);
    }

    // setup buffers for instancing rendering 
    if (auxBuffer)
    {
        lite3d_vbo_bind(auxBuffer);
        for(i = 0; i < 4; ++i)
        {
            glEnableVertexAttribArray(attribIndex);
            glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE, sizeof(kmMat4), LITE3D_BUFFER_OFFSET(i * sizeof(kmVec4)));
            glVertexAttribDivisor(attribIndex++, 1);
        }
    }

    if (indexesCount > 0 && indexesSize > 0 && indexBuffer)
    {
        lite3d_vbo_bind(indexBuffer);
    }

    /* end VAO binding */
    lite3d_vao_unbind();

    vao->elementsCount = (indexesCount > 0 ? indexesCount : verticesCount) / 3;
    vao->indexesOffset = indexesOffset;
    vao->indexesCount = indexesCount;
    vao->indexesSize = indexesSize;
    vao->verticesCount = verticesCount;
    vao->verticesSize = verticesSize;
    vao->verticesOffset = verticesOffset;
    return LITE3D_TRUE;
}
