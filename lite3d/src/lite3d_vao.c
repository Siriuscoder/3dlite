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
#include <lite3d/lite3d_vao.h>

static int instancingSupport;

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

    glDrawElements(GL_TRIANGLES, vao->indexesCount, vao->indexType, LITE3D_BUFFER_OFFSET(vao->indexesOffset));
}

void lite3d_vao_draw_indexed_instanced(struct lite3d_vao *vao, size_t count)
{
    /* glDrawElementsInstanced behaves identically to glDrawElements 
     * except that primcount instances of the set of elements are executed. 
     * Those attributes that have divisor N where N is other than zero 
     * (as specified by glVertexAttribDivisor) advance once every N instances. 
     */

    if (!instancingSupport)
        return;
    SDL_assert(vao);
    glDrawElementsInstanced(GL_TRIANGLES, vao->indexesCount,
        vao->indexType, (void *) vao->indexesOffset, count);
}

void lite3d_vao_draw(struct lite3d_vao *vao)
{
    glDrawArrays(GL_TRIANGLES, 0, vao->verticesCount);
}

void lite3d_vao_draw_instanced(struct lite3d_vao *vao, size_t count)
{
    if (!instancingSupport)
        return;
    SDL_assert(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0,
        vao->verticesCount, count);
}

void lite3d_vao_bind(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    /* bind current vao */
    glBindVertexArray(vao->vaoID);
}

void lite3d_vao_unbind(struct lite3d_vao *vao)
{
    /* zero bind */
    glBindVertexArray(0);
}

int lite3d_vao_init(struct lite3d_vao *vao)
{
    SDL_assert(vao);

    memset(vao, 0, sizeof (lite3d_vao));

    lite3d_misc_gl_error_stack_clean();
    glGenVertexArrays(1, &vao->vaoID);

    return !lite3d_misc_check_gl_error();
}

void lite3d_vao_purge(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    glDeleteVertexArrays(1, &vao->vaoID);
    vao->vaoID = 0;
}

