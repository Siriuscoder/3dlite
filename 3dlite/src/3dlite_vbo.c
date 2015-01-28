/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
*
*	Foobar is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	Foobar is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_vbo.h>


int lite3d_vbo_technique_init(void)
{
    if (!GL_VERSION_3_1)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL v3.1 minimum required (VBO)", __FUNCTION__);
        return LITE3D_FALSE;
    }

    if (!GL_ARB_vertex_buffer_object)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL_ARB_vertex_buffer_object not supported..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    if (!GL_ARB_vertex_array_object)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL_ARB_vertex_array_object not supported..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

int lite3d_vbo_init(struct lite3d_vbo *vbo)
{
    SDL_assert(vbo);

    memset(vbo, 0, sizeof(lite3d_vbo));
    lite3d_list_init(&vbo->vaos);

    lite3d_misc_gl_error_stack_clean();
    /* gen buffer for store vertex data */
    glGenBuffers(1, &vbo->vboVerticesID);
    if(!lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    glGenBuffers(1, &vbo->vboIndexesID);
    if(!lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

void lite3d_vbo_purge(struct lite3d_vbo *vbo)
{
    lite3d_list_node *vaoLink;
    SDL_assert(vbo);

    for (vaoLink = vbo->vaos.l.next; 
        vaoLink != &vbo->vaos.l; vaoLink = lite3d_list_next(vaoLink))
    {
        lite3d_vao_purge(MEMBERCAST(lite3d_vao, vaoLink, inVbo));
    }

    glDeleteBuffers(1, &vbo->vboVerticesID);
    glDeleteBuffers(1, &vbo->vboIndexesID);
}

void lite3d_vbo_draw(struct lite3d_vbo *vbo)
{
    lite3d_list_node *vaoLink;
    lite3d_vao *vao;
    SDL_assert(vbo);

    if(vbo->bindOn)
        vbo->bindOn(vbo);

    for (vaoLink = vbo->vaos.l.next; 
        vaoLink != &vbo->vaos.l; vaoLink = lite3d_list_next(vaoLink))
    {
        vao = MEMBERCAST(lite3d_vao, vaoLink, inVbo);

        if(vao->batchBegin)
            vao->batchBegin(vbo, vao);
        lite3d_vao_draw(vao);
        if(vao->batchEnd)
            vao->batchEnd(vbo, vao);
    }

    if(vbo->bindOff)
        vbo->bindOff(vbo);
}

void lite3d_vao_draw(struct lite3d_vao *vao)
{
    /* activate current meterial */
    /* bind current vao */
    glBindVertexArray(vao->vaoID);
    glDrawElements(GL_TRIANGLES, vao->elementsCount, vao->indexType, (void *)vao->offsetIndexes);
    glBindVertexArray(0);
}

int lite3d_vao_init(struct lite3d_vao *vao)
{
    SDL_assert(vao);

    memset(vao, 0, sizeof(lite3d_vao));
    lite3d_list_link_init(&vao->inVbo);

    lite3d_misc_gl_error_stack_clean();
    glGenVertexArrays(1, &vao->vaoID);

    return lite3d_misc_check_gl_error();
}

void lite3d_vao_purge(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    glDeleteVertexArrays(1, &vao->vaoID);
}
