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
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_mesh.h>

static lite3d_vbo gAuxGlobalBuffer = {0, 0, 0};

int lite3d_mesh_init(struct lite3d_mesh *mesh)
{
    SDL_assert(mesh);

    memset(mesh, 0, sizeof (lite3d_mesh));
    mesh->version = LITE3D_VERSION_NUM;
    lite3d_array_init(&mesh->chunks, sizeof (lite3d_mesh_chunk), 1);
    
    if (lite3d_check_instanced_arrays())
    {
        if (!gAuxGlobalBuffer.vboID)
        {
            if (!lite3d_vbo_init(&gAuxGlobalBuffer))
                return LITE3D_FALSE;
        }

        mesh->auxBuffer = &gAuxGlobalBuffer;
    }

    /* gen buffer for store vertex data */
    if (!lite3d_vbo_init(&mesh->vertexBuffer))
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    if (!lite3d_ibo_init(&mesh->indexBuffer))
    {
        lite3d_vbo_purge(&mesh->vertexBuffer);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_mesh_purge(struct lite3d_mesh *mesh)
{
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    LITE3D_ARR_FOREACH(&mesh->chunks, lite3d_mesh_chunk, meshChunk)
    {
        lite3d_mesh_chunk_purge(meshChunk);
    }

    lite3d_array_purge(&mesh->chunks);
    lite3d_vbo_purge(&mesh->vertexBuffer);
    lite3d_vbo_purge(&mesh->indexBuffer);
}

int lite3d_mesh_extend(struct lite3d_mesh *mesh, size_t verticesSize,
    size_t indexesSize, uint16_t access)
{
    SDL_assert(mesh);

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

void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    
    if (meshChunk->hasIndexes)
        lite3d_vao_draw_indexed(&meshChunk->vao);
    else
        lite3d_vao_draw(&meshChunk->vao);
}

void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t instancesCount)
{
    SDL_assert(meshChunk);
    
    if (instancesCount == 0)
        return;

    if (meshChunk->hasIndexes)
        lite3d_vao_draw_indexed_instanced(&meshChunk->vao, instancesCount);
    else
        lite3d_vao_draw_instanced(&meshChunk->vao, instancesCount);
}

void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_bind(&meshChunk->vao);
}

void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_unbind(&meshChunk->vao);
}

int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk, uint8_t indexed)
{
    SDL_assert(meshChunk);

    memset(meshChunk, 0, sizeof (lite3d_mesh_chunk));
    meshChunk->hasIndexes = indexed;

    return lite3d_vao_init(&meshChunk->vao);
}

void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk)
{
    SDL_assert(meshChunk);
    lite3d_vao_purge(&meshChunk->vao);
    if (meshChunk->layoutEntriesCount > 0)
        lite3d_free(meshChunk->layout);
}

lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_material_index(struct lite3d_mesh *mesh,
    uint32_t materialIndex)
{
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    LITE3D_ARR_FOREACH(&mesh->chunks, lite3d_mesh_chunk, meshChunk)
    {
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
    lite3d_mesh_chunk meshChunk;
    if (!lite3d_mesh_chunk_init(&meshChunk, LITE3D_TRUE))
    {
        return NULL;
    }

    meshChunk.mesh = mesh;
    meshChunk.layout = (lite3d_vao_layout *) lite3d_malloc(sizeof (lite3d_vao_layout) * layoutCount);
    if (!meshChunk.layout)
    {
        lite3d_mesh_chunk_purge(&meshChunk);
        return NULL;
    }

    memcpy(meshChunk.layout, layout, layoutCount * sizeof(lite3d_vao_layout));

    if (!lite3d_vao_init_layout(&mesh->vertexBuffer, &mesh->indexBuffer, mesh->auxBuffer, 
        &meshChunk.vao, meshChunk.layout, layoutCount, stride, indexesCount, 
        indexesSize, indexesOffset, verticesCount, verticesSize, verticesOffset))
    {
        lite3d_mesh_chunk_purge(&meshChunk);
        return NULL;
    }

    meshChunk.vertexStride = stride;
    meshChunk.layoutEntriesCount = layoutCount;
    meshChunk.hasIndexes = indexesCount > 0 && indexesSize > 0 ? LITE3D_TRUE : LITE3D_FALSE;

    LITE3D_ARR_ADD_ELEM(&mesh->chunks, lite3d_mesh_chunk, meshChunk);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: 0x%016llx: chunk 0x%016llx: %s, cv/ov/sv %d/%zub/%udb, ci/oi %d/%zub",
        (unsigned long long)mesh, (unsigned long long)LITE3D_ARR_GET_LAST(&mesh->chunks, lite3d_mesh_chunk), "TRIANGLES",
        meshChunk.vao.verticesCount, meshChunk.vao.verticesOffset, stride, meshChunk.vao.indexesCount, meshChunk.vao.indexesOffset);

    return LITE3D_ARR_GET_LAST(&mesh->chunks, lite3d_mesh_chunk);
}

