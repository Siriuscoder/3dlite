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
#include <3dlite/3dlite_mesh.h>


int lite3d_indexed_mesh_init(struct lite3d_indexed_mesh *mesh)
{
    SDL_assert(mesh);

    memset(mesh, 0, sizeof (lite3d_indexed_mesh));
    lite3d_list_init(&mesh->chunks);

    /* gen buffer for store vertex data */
    if(!lite3d_vbo_init(&mesh->vertexBuffer))
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    if(!lite3d_vbo_init(&mesh->indexBuffer))
    {
        lite3d_vbo_purge(&mesh->vertexBuffer);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_indexed_mesh_purge(struct lite3d_indexed_mesh *mesh)
{
    lite3d_list_node *vaoLink;
    SDL_assert(mesh);

    while ((vaoLink = lite3d_list_first_link(&mesh->chunks)) != NULL)
    {
        lite3d_list_unlink_link(vaoLink);
        lite3d_mesh_chunk_purge(LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node));
    }

    lite3d_vbo_purge(&mesh->vertexBuffer);
    lite3d_vbo_purge(&mesh->indexBuffer);

    mesh->chunkCount = 0;
}

void lite3d_indexed_mesh_draw(struct lite3d_indexed_mesh *mesh)
{
    lite3d_list_node *vaoLink;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        lite3d_indexed_mesh_chunk_draw(meshChunk);
    }
}

void lite3d_indexed_mesh_draw_instanced(struct lite3d_indexed_mesh *mesh, size_t count)
{
    lite3d_list_node *vaoLink;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        lite3d_indexed_mesh_chunk_draw_instanced(meshChunk, count);
    }
}

int lite3d_indexed_mesh_extend(struct lite3d_indexed_mesh *mesh, size_t verticesSize,
    size_t indexesSize, uint16_t access)
{
    SDL_assert(mesh);

    lite3d_misc_gl_error_stack_clean();

    if (verticesSize > 0)
    {
        if (!lite3d_vbo_extend(&mesh->vertexBuffer, verticesSize, access))
            return LITE3D_FALSE;
    }
    if (indexesSize > 0)
    {
        if (!lite3d_vbo_extend(&mesh->indexBuffer, indexesSize, access))
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}


void lite3d_indexed_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk)
{
    lite3d_vao_draw_indexed(&meshChunk->vao);
}

void lite3d_indexed_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t count)
{
    lite3d_vao_draw_indexed_instanced(&meshChunk->vao, count);
}

void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk)
{
    lite3d_vao_draw(&meshChunk->vao);
}

void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t count)
{
    lite3d_vao_draw_instanced(&meshChunk->vao, count);
}

void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk)
{
    lite3d_vao_bind(&meshChunk->vao);
}

void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk)
{
    lite3d_vao_unbind(&meshChunk->vao);
}

int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);

    memset(meshChunk, 0, sizeof (lite3d_mesh_chunk));
    lite3d_list_link_init(&meshChunk->node);

    return lite3d_vao_init(&meshChunk->vao);
}

void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_purge(&meshChunk->vao);
    lite3d_free_pooled(LITE3D_POOL_NO1, meshChunk);
}

lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_index(struct lite3d_indexed_mesh *mesh,
    uint32_t materialIndex)
{
    lite3d_list_node *vaoLink;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        if (meshChunk->materialIndex == materialIndex)
            return meshChunk;
    }

    return NULL;
}
