/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_mesh_codec.h>

int lite3d_indexed_mesh_load_from_memory(lite3d_indexed_mesh *mesh,
    const void *vertices,
    size_t verticesCount,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes,
    size_t elementsCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i;
    uint8_t componentSize;
    uint16_t componentType;
    uint16_t indexPrimitive;

    SDL_assert(mesh && layout);

    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = stride * verticesCount;

    /* store vertex data to GPU memory */
    if (!lite3d_vbo_buffer(&mesh->vertexBuffer, vertices, verticesSize, access))
        return LITE3D_FALSE;

    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * elementsCount;
    /* store index data to GPU memory */
    if (!lite3d_vbo_buffer(&mesh->indexBuffer, indexes, indexesSize, access))
        return LITE3D_FALSE;

    componentType = lite3d_index_component_type_by_size(componentSize);
    indexPrimitive = lite3d_index_primitive_by_components(indexComponents);

    /* append new batch */
    if (!lite3d_indexed_mesh_append_chunk(mesh, layout, layoutCount, stride,
        componentType, indexPrimitive, elementsCount * indexComponents,
        indexesSize, 0, verticesCount, verticesSize, 0))
        return LITE3D_FALSE;

    mesh->verticesCount = verticesCount;
    mesh->elementsCount = elementsCount;

    return LITE3D_TRUE;
}

int lite3d_indexed_mesh_extend_from_memory(lite3d_indexed_mesh *mesh,
    const void *vertices,
    size_t verticesCount,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes,
    size_t elementsCount,
    uint8_t indexComponents,
    uint16_t access)
{
    size_t verticesSize = 0, indexesSize = 0,
        stride = 0, i, offsetVertices = 0,
        offsetIndexes = 0;
    uint8_t componentSize;
    uint16_t componentType;
    uint16_t indexPrimitive;

    SDL_assert(mesh && layout);

    if (lite3d_list_is_empty(&mesh->chunks))
        return lite3d_indexed_mesh_load_from_memory(mesh, vertices, verticesCount,
        layout, layoutCount, indexes, elementsCount, indexComponents, access);

    /* calculate buffer parameters */
    for (i = 0; i < layoutCount; ++i)
        stride += layout[i].count * sizeof (float);
    verticesSize = stride * verticesCount;
    componentSize = verticesCount <= 0xff ? 1 : (verticesCount <= 0xffff ? 2 : 4);
    indexesSize = indexComponents * componentSize * elementsCount;
    /* expand VBO */
    offsetVertices = mesh->vertexBuffer.size;
    offsetIndexes = mesh->indexBuffer.size;
    if (!lite3d_indexed_mesh_extend(mesh, verticesSize, indexesSize, access))
        return LITE3D_FALSE;

    /* copy vertices to the end of the vertex buffer */
    if (!lite3d_vbo_subbuffer(&mesh->vertexBuffer, vertices,
        offsetVertices, verticesSize))
        return LITE3D_FALSE;

    /* copy indexes to the end of the index buffer */
    if (!lite3d_vbo_subbuffer(&mesh->indexBuffer, indexes,
        offsetIndexes, indexesSize))
        return LITE3D_FALSE;

    componentType = lite3d_index_component_type_by_size(componentSize);
    indexPrimitive = lite3d_index_primitive_by_components(indexComponents);

    /* append new batch */
    if (!lite3d_indexed_mesh_append_chunk(mesh, layout, layoutCount, stride,
        componentType, indexPrimitive, elementsCount * indexComponents,
        indexesSize, offsetIndexes, verticesCount, verticesSize, offsetVertices))
        return LITE3D_FALSE;

    mesh->verticesCount += verticesCount;
    mesh->elementsCount += elementsCount;

    return LITE3D_TRUE;
}

void lite3d_indexed_mesh_order_mat_indexes(lite3d_indexed_mesh *mesh)
{
    lite3d_list_node *vaoLink;
    uint32_t materialIndex = 0;
    SDL_assert(mesh);

    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node)->
            materialIndex = materialIndex++;
    }
}

int lite3d_indexed_mesh_load_from_m_file(lite3d_indexed_mesh *mesh, const lite3d_file *resource,
    uint16_t access)
{
    if (!resource->isLoaded)
        return LITE3D_FALSE;

    return lite3d_indexed_mesh_m_decode(mesh, resource->fileBuff, resource->fileSize,
        access);
}

