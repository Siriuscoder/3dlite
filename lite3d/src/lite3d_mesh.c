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
#include <lite3d/lite3d_mesh.h>

#include <string.h>
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_shader_program.h>

static lite3d_vbo gAuxGlobalBuffer = {0, 0, 0};

int lite3d_mesh_aux_buffer_init(void)
{
    if (lite3d_check_instanced_arrays())
    {
        if (!gAuxGlobalBuffer.vboID)
        {
            if (!lite3d_vbo_init(&gAuxGlobalBuffer, LITE3D_VBO_STREAM_DRAW))
            {
                return LITE3D_FALSE;
            }
        }

        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

int lite3d_mesh_init(struct lite3d_mesh *mesh, uint16_t usage)
{
    SDL_assert(mesh);

    memset(mesh, 0, sizeof (lite3d_mesh));
    mesh->version = LITE3D_VERSION_NUM;
    lite3d_list_init(&mesh->chunks);

    if (gAuxGlobalBuffer.vboID != 0)
    {
        mesh->auxBuffer = &gAuxGlobalBuffer;
    }
    
    /* gen buffer for store vertex data */
    if (!lite3d_vbo_init(&mesh->vertexBuffer, usage))
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    if (!lite3d_ibo_init(&mesh->indexBuffer, usage))
    {
        lite3d_vbo_purge(&mesh->vertexBuffer);
        return LITE3D_FALSE;
    }

    if (lite3d_check_multi_draw_indirect())
    {
        if (!lite3d_vbo_indirect_init(&mesh->indirectBuffer, LITE3D_VBO_STREAM_DRAW))
        {
            lite3d_vbo_purge(&mesh->vertexBuffer);
            lite3d_vbo_purge(&mesh->indexBuffer);
            return LITE3D_FALSE;
        }
    }

    return LITE3D_TRUE;
}

void lite3d_mesh_purge(struct lite3d_mesh *mesh)
{
    lite3d_list_node *link = NULL;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    while ((link = lite3d_list_remove_first_link(&mesh->chunks)) != NULL)
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);
        lite3d_mesh_chunk_purge(meshChunk);
        lite3d_free_pooled(LITE3D_POOL_NO3, meshChunk);
    }

    lite3d_vbo_purge(&mesh->vertexBuffer);
    lite3d_vbo_purge(&mesh->indexBuffer);
    lite3d_vbo_purge(&mesh->indirectBuffer);
    lite3d_array_purge(&mesh->drawQueue);
}

int lite3d_mesh_extend(struct lite3d_mesh *mesh, size_t verticesSize,
    size_t indexesSize)
{
    SDL_assert(mesh);

    if (verticesSize > 0)
    {
        if (!lite3d_vbo_extend(&mesh->vertexBuffer, verticesSize))
            return LITE3D_FALSE;
    }
    if (indexesSize > 0)
    {
        if (!lite3d_vbo_extend(&mesh->indexBuffer, indexesSize))
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);

    lite3d_mesh_chunk_bind(meshChunk);
    if (lite3d_shader_program_validate_current())
    {
        if (meshChunk->hasIndexes)
            lite3d_vao_draw_indexed(&meshChunk->vao);
        else
            lite3d_vao_draw(&meshChunk->vao);
    }
}

void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, uint32_t instancesCount)
{
    SDL_assert(meshChunk);
    
    if (instancesCount == 0)
        return;

    lite3d_mesh_chunk_bind(meshChunk);
    if (lite3d_shader_program_validate_current())
    {
        if (meshChunk->hasIndexes)
            lite3d_vao_draw_indexed_instanced(&meshChunk->vao, instancesCount);
        else
            lite3d_vao_draw_instanced(&meshChunk->vao, instancesCount);
    }
}

void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_bind(&meshChunk->vao);
}

void lite3d_mesh_chunk_unbind(void)
{
    lite3d_vao_unbind();
}

int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk, size_t layoutCount)
{
    SDL_assert(meshChunk);

    memset(meshChunk, 0, sizeof (lite3d_mesh_chunk));
    lite3d_list_link_init(&meshChunk->link);
    lite3d_array_init(&meshChunk->layout, sizeof(lite3d_vao_layout), layoutCount);
    meshChunk->hasIndexes = LITE3D_TRUE;

    return lite3d_vao_init(&meshChunk->vao);
}

void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_purge(&meshChunk->vao);
    lite3d_array_purge(&meshChunk->layout);
}

lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_material_index(struct lite3d_mesh *mesh,
    uint32_t materialIndex)
{
    lite3d_list_node *link;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    for (link = mesh->chunks.l.next; link != &mesh->chunks.l; link = lite3d_list_next(link))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);
        if (meshChunk->materialIndex == materialIndex)
            return meshChunk;
    }

    return NULL;
}

lite3d_mesh_chunk *lite3d_mesh_append_chunk(lite3d_mesh *mesh,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount,
    uint32_t stride,
    uint32_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    uint32_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset)
{
    lite3d_mesh_chunk *meshChunk;

    SDL_assert(layoutCount > 0);
    SDL_assert(layout);

    meshChunk = lite3d_malloc_pooled(LITE3D_POOL_NO3, sizeof(lite3d_mesh_chunk));
    if (!meshChunk)
    {
        return NULL;
    }

    if (!lite3d_mesh_chunk_init(meshChunk, layoutCount))
    {
        return NULL;
    }

    meshChunk->mesh = mesh;
    for (uint32_t i = 0; i < layoutCount; ++i)
    {
        LITE3D_ARR_ADD_ELEM(&meshChunk->layout, lite3d_vao_layout, layout[i]);
    }

    if (!lite3d_vao_init_layout(&mesh->vertexBuffer, &mesh->indexBuffer, mesh->auxBuffer, 
        &meshChunk->vao, meshChunk->layout.data, layoutCount, stride, indexesCount, 
        indexesSize, indexesOffset, verticesCount, verticesSize, verticesOffset))
    {
        lite3d_mesh_chunk_purge(meshChunk);
        lite3d_free_pooled(LITE3D_POOL_NO3, meshChunk);
        return NULL;
    }

    meshChunk->vertexStride = stride;
    meshChunk->hasIndexes = indexesCount > 0 && indexesSize > 0 ? LITE3D_TRUE : LITE3D_FALSE;

    lite3d_list_add_last_link(&meshChunk->link, &mesh->chunks);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: 0x%016llx: chunk 0x%016llx: %s, cv/ov/sv %d/%zub/%udb, ci/oi %d/%zub",
        (unsigned long long)mesh, 
        (unsigned long long)meshChunk, 
        "TRIANGLES",
        meshChunk->vao.verticesCount, 
        meshChunk->vao.verticesOffset, 
        stride, 
        meshChunk->vao.indexesCount, 
        meshChunk->vao.indexesOffset);

    return meshChunk;
}

void lite3d_mesh_queue_draw(struct lite3d_mesh *mesh)
{
    lite3d_mesh_chunk *firstchunk;
    SDL_assert(mesh);

    firstchunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_first_link(&mesh->chunks), link);
    if (!lite3d_vbo_buffer_set(&mesh->indirectBuffer, mesh->drawQueue.data, 
        mesh->drawQueue.size * mesh->drawQueue.elemSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unable to write to the draw indirect buffer",
            LITE3D_CURRENT_FUNCTION);
        return;
    }

    lite3d_mesh_chunk_bind(firstchunk);
    // Проверим впорядке ли шейдер
    if (lite3d_shader_program_validate_current())
    {
        if (firstchunk->hasIndexes)
        {
            lite3d_vao_multidraw_indexed(0, mesh->drawQueue.size);
        }
        else
        {
            lite3d_vao_multidraw(0, mesh->drawQueue.size);
        }
    }
}

void lite3d_mesh_queue_chunk(struct lite3d_mesh_chunk *meshChunk, uint32_t instancesCount)
{
    SDL_assert(meshChunk);
    SDL_assert(meshChunk->mesh);

    if (meshChunk->hasIndexes)
    {
        if (meshChunk->mesh->drawQueue.capacity == 0)
        {
            lite3d_array_init(&meshChunk->mesh->drawQueue, sizeof(lite3d_multidraw_indexed_command), 1);
        }

        lite3d_multidraw_indexed_command *command = lite3d_array_add(&meshChunk->mesh->drawQueue);
        command->count = meshChunk->vao.indexesCount;
        command->instanceCount = instancesCount;
        command->firstIndex = (uint32_t)(meshChunk->vao.indexesOffset / sizeof(uint32_t));
        command->baseVertex = (uint32_t)(meshChunk->vao.verticesOffset / meshChunk->vertexStride);
        command->baseInstance = 0;
    }
    else
    {
        if (meshChunk->mesh->drawQueue.capacity == 0)
        {
            lite3d_array_init(&meshChunk->mesh->drawQueue, sizeof(lite3d_multidraw_command), 1);
        }

        lite3d_multidraw_command *command = lite3d_array_add(&meshChunk->mesh->drawQueue);
        command->count = meshChunk->vao.verticesCount;
        command->instanceCount = instancesCount;
        command->first = (uint32_t)(meshChunk->vao.verticesOffset / meshChunk->vertexStride);
        command->baseInstance = 0;
    }
}

void lite3d_mesh_queue_clean(struct lite3d_mesh *mesh)
{
    SDL_assert(mesh);
    lite3d_array_clean(&mesh->drawQueue);
}

