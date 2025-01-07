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

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_codec.h>

int lite3d_mesh_indexed_load_from_memory(lite3d_mesh *mesh,
    const void *vertices,
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount,
    const void *indexes,
    uint32_t elementsCount)
{
    size_t verticesSize = 0, indexesSize = 0;
    uint32_t stride = 0, i;

    SDL_assert(mesh && layout);

    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = (size_t)stride * (size_t)verticesCount;

    /* store vertex data to GPU memory */
    if (!lite3d_vbo_buffer_alloc(&mesh->vertexBuffer, vertices, verticesSize))
        return LITE3D_FALSE;

    indexesSize = 3 * sizeof(uint32_t) * elementsCount;
    /* store index data to GPU memory */
    if (!lite3d_vbo_buffer_alloc(&mesh->indexBuffer, indexes, indexesSize))
        return LITE3D_FALSE;

    /* append new batch */
    if (!lite3d_mesh_append_chunk(mesh, layout, layoutCount, stride, elementsCount * 3,
        indexesSize, 0, verticesCount, verticesSize, 0))
        return LITE3D_FALSE;

    mesh->verticesCount = verticesCount;
    mesh->elementsCount = elementsCount;

    return LITE3D_TRUE;
}

int lite3d_mesh_indexed_append_from_memory(lite3d_mesh *mesh,
    const void *vertices,
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount,
    const void *indexes,
    uint32_t elementsCount)
{
    size_t verticesSize = 0, indexesSize = 0, offsetVertices = 0, offsetIndexes = 0;
    size_t verticesExpandSize = 0, indexExpandSize = 0;
    uint32_t stride = 0, i;


    SDL_assert(mesh && layout);

    if (!lite3d_list_is_empty(&mesh->chunks))
    {
        lite3d_mesh_chunk *lastChunk;
        lastChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&mesh->chunks), link);
        offsetVertices = lastChunk->vao.verticesOffset + lastChunk->vao.verticesSize;
        offsetIndexes = lastChunk->vao.indexesOffset + lastChunk->vao.indexesSize;
    }

    /* calculate buffer parameters */
    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = (size_t)stride * (size_t)verticesCount;
    indexesSize = 3 * sizeof(uint32_t) * elementsCount;

    /* expand VBOs */
    verticesExpandSize = offsetVertices + verticesSize > mesh->vertexBuffer.size ? 
        offsetVertices + verticesSize - mesh->vertexBuffer.size : 0;
    indexExpandSize = offsetIndexes + indexesSize > mesh->indexBuffer.size ? 
        offsetIndexes + indexesSize - mesh->indexBuffer.size : 0;

    if (!lite3d_mesh_extend(mesh, verticesExpandSize, indexExpandSize))
        return LITE3D_FALSE;

    /* copy vertices to the end of the vertex buffer */
    if (!lite3d_vbo_subbuffer(&mesh->vertexBuffer, vertices,
        offsetVertices, verticesSize))
        return LITE3D_FALSE;

    /* copy indexes to the end of the index buffer */
    if (!lite3d_vbo_subbuffer(&mesh->indexBuffer, indexes,
        offsetIndexes, indexesSize))
        return LITE3D_FALSE;

    /* append new batch */
    if (!lite3d_mesh_append_chunk(mesh, layout, layoutCount, stride, elementsCount * 3,
        indexesSize, offsetIndexes, verticesCount, verticesSize, offsetVertices))
        return LITE3D_FALSE;

    mesh->verticesCount += verticesCount;
    mesh->elementsCount += elementsCount;

    return LITE3D_TRUE;
}

int lite3d_mesh_load_from_memory(lite3d_mesh *mesh,
    const void *vertices,
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount)
{
    size_t verticesSize = 0;
    uint32_t stride = 0, i;

    SDL_assert(mesh && layout);

    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = (size_t)stride * (size_t)verticesCount;

    /* store vertex data to GPU memory */
    if (!lite3d_vbo_buffer_alloc(&mesh->vertexBuffer, vertices, verticesSize))
        return LITE3D_FALSE;

    /* append new batch */
    if (!lite3d_mesh_append_chunk(mesh, layout, layoutCount, stride,
        0, 0, 0, verticesCount, verticesSize, 0))
        return LITE3D_FALSE;

    mesh->verticesCount = verticesCount;

    return LITE3D_TRUE;
}

int lite3d_mesh_append_from_memory(lite3d_mesh *mesh,
    const void *vertices,
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount)
{
    size_t verticesSize = 0, offsetVertices = 0;
    uint32_t stride = 0, i;

    SDL_assert(mesh && layout);

    if (!lite3d_list_is_empty(&mesh->chunks))
    {
        lite3d_mesh_chunk *lastChunk;
        lastChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&mesh->chunks), link);
        offsetVertices = lastChunk->vao.verticesOffset + lastChunk->vao.verticesSize;
    }

    /* calculate buffer parameters */
    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = (size_t)stride * (size_t)verticesCount;
    /* expand VBO */
    if (offsetVertices + verticesSize > mesh->vertexBuffer.size)
    {
        if (!lite3d_mesh_extend(mesh, offsetVertices + verticesSize - mesh->vertexBuffer.size, 0))
            return LITE3D_FALSE;
    }

    /* copy vertices to the end of the vertex buffer */
    if (!lite3d_vbo_subbuffer(&mesh->vertexBuffer, vertices,
        offsetVertices, verticesSize))
        return LITE3D_FALSE;

    /* append new batch */
    if (!lite3d_mesh_append_chunk(mesh, layout, layoutCount, stride,
        0, 0, 0, verticesCount, verticesSize, offsetVertices))
        return LITE3D_FALSE;

    mesh->verticesCount += verticesCount;

    return LITE3D_TRUE;
}

void lite3d_mesh_order_mat_indexes(lite3d_mesh *mesh)
{
    lite3d_list_node *link;
    lite3d_mesh_chunk *meshChunk;
    uint32_t materialIndex = 0;
    SDL_assert(mesh);

    for (link = mesh->chunks.l.next; link != &mesh->chunks.l; link = lite3d_list_next(link))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);
        meshChunk->materialIndex = materialIndex++;
    }
}

int lite3d_mesh_load_from_m_file(lite3d_mesh *mesh, const lite3d_file *resource)
{
    if (!resource->isLoaded)
        return LITE3D_FALSE;

    return lite3d_mesh_m_decode(mesh, resource->fileBuff, resource->fileSize);
}

