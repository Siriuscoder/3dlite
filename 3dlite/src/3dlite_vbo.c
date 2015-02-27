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

static int maxVertexAttribs;

static int vbo_expand(uint32_t *vboID, size_t expandSize, uint16_t access)
{
    int32_t originSize;
    uint32_t newVbo;

    glBindBuffer(GL_COPY_READ_BUFFER, *vboID);
    glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &originSize);
    
    glGenBuffers(1, &newVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, newVbo);
    /* allocate new buffer */
    glBufferData(GL_COPY_WRITE_BUFFER, originSize + expandSize, NULL, access);
    /* copy data to new buffer */
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, originSize);
    
    if(lite3d_misc_check_gl_error())
    {
        glDeleteBuffers(1, &newVbo);;
        return LITE3D_FALSE;
    }
    
    glDeleteBuffers(1, vboID);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

    *vboID = newVbo;
    return LITE3D_TRUE;
}

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
    
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Max vertex attributes: %d",
        maxVertexAttribs);
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
    if(lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    glGenBuffers(1, &vbo->vboIndexesID);
    if(lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

void lite3d_vbo_purge(struct lite3d_vbo *vbo)
{
    lite3d_list_node *vaoLink;
    SDL_assert(vbo);

    while((vaoLink = lite3d_list_first_link(&vbo->vaos)) != NULL)
    {
        lite3d_list_unlink_link(vaoLink);
        lite3d_vao_purge(MEMBERCAST(lite3d_vao, vaoLink, inVbo));
    }

    glDeleteBuffers(1, &vbo->vboVerticesID);
    glDeleteBuffers(1, &vbo->vboIndexesID);

    vbo->vboVerticesID = vbo->vboIndexesID = 0;
    vbo->vaosCount = 0;
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

int lite3d_vbo_extend(struct lite3d_vbo *vbo, size_t verticesSize,
    size_t indexesSize, uint16_t access)
{
    SDL_assert(vbo);
    if(!GLEW_ARB_copy_buffer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLEW_ARB_copy_buffer not supported..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    lite3d_misc_gl_error_stack_clean();

    if(verticesSize > 0)
    {
        if(!vbo_expand(&vbo->vboVerticesID, verticesSize, access))
            return LITE3D_FALSE;
    }
    if(indexesSize > 0)
    {
        if(!vbo_expand(&vbo->vboIndexesID, indexesSize, access))
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_vao_draw(struct lite3d_vao *vao)
{
    /* activate current meterial */
    /* bind current vao */
    glBindVertexArray(vao->vaoID);
    glDrawElements(vao->elementType, vao->indexesCount, vao->indexType, (void *)vao->indexesOffset);
    glBindVertexArray(0);
}

int lite3d_vao_init(struct lite3d_vao *vao)
{
    SDL_assert(vao);

    memset(vao, 0, sizeof(lite3d_vao));
    lite3d_list_link_init(&vao->inVbo);

    lite3d_misc_gl_error_stack_clean();
    glGenVertexArrays(1, &vao->vaoID);

    return !lite3d_misc_check_gl_error();
}

void lite3d_vao_purge(struct lite3d_vao *vao)
{
    SDL_assert(vao);
    glDeleteVertexArrays(1, &vao->vaoID);
    lite3d_free(vao);
}

lite3d_vao *lite3d_vao_get_by_index(struct lite3d_vbo *vbo,
    uint32_t materialIndex)
{
    lite3d_list_node *vaoLink;
    lite3d_vao *vao;
    SDL_assert(vbo);

    for (vaoLink = vbo->vaos.l.next; 
        vaoLink != &vbo->vaos.l; vaoLink = lite3d_list_next(vaoLink))
    {
        vao = MEMBERCAST(lite3d_vao, vaoLink, inVbo);
        if(vao->materialIndex == materialIndex)
            return vao;
    }

    return NULL;
}
