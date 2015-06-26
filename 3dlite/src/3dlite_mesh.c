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
    if (!lite3d_vbo_init(&mesh->vertexBuffer))
        return LITE3D_FALSE;

    /* gen buffer for store index data */
    if (!lite3d_vbo_init(&mesh->indexBuffer))
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
    if (meshChunk->layoutEntriesCount > 0)
        lite3d_free(meshChunk->layout);
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

lite3d_mesh_chunk *lite3d_indexed_mesh_append_chunk(lite3d_indexed_mesh *mesh,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    size_t stride,
    uint16_t componentType,
    uint16_t indexPrimitive,
    size_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    size_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset)
{
    lite3d_mesh_chunk *meshChunk;
    uint32_t attribIndex = 0;
    size_t i = 0;
    size_t vOffset = verticesOffset;

    meshChunk = (lite3d_mesh_chunk *) lite3d_malloc_pooled(LITE3D_POOL_NO1, sizeof (lite3d_mesh_chunk));
    SDL_assert_release(meshChunk);

    if (!lite3d_mesh_chunk_init(meshChunk))
    {
        lite3d_free_pooled(LITE3D_POOL_NO1, meshChunk);
        return NULL;
    }

    meshChunk->layout = (lite3d_indexed_mesh_layout *) lite3d_malloc(sizeof (lite3d_indexed_mesh_layout) * layoutCount);
    SDL_assert_release(meshChunk->layout);

    /* VAO set current */
    glBindVertexArray(meshChunk->vao.vaoID);
    /* use single VBO to store all data */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer.vboID);
    /* bind all arrays and attribs into the current VAO */
    for (; i < layoutCount; ++i)
    {
        if (layout[i].binding != LITE3D_BUFFER_BINDING_ATTRIBUTE)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "VBO: 0x%x: chunk 0x%x used "
                "legacy binding type attribute", mesh->vertexBuffer.vboID, meshChunk->vao.vaoID);
            continue;
        }

        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex++, layout[i].count, GL_FLOAT,
            GL_FALSE, stride, (void *) vOffset);

        vOffset += layout[i].count * sizeof (GLfloat);
        meshChunk->layout[i] = layout[i];
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer.vboID);
    /* end VAO binding */
    glBindVertexArray(0);

    meshChunk->vao.indexesOffset = indexesOffset;
    meshChunk->vao.indexType = componentType;
    meshChunk->vao.elementType = indexPrimitive;
    meshChunk->vao.indexesCount = indexesCount;
    meshChunk->vao.indexesSize = indexesSize;
    meshChunk->vao.verticesCount = verticesCount;
    meshChunk->vao.verticesSize = verticesSize;
    meshChunk->vao.verticesOffset = verticesOffset;
    meshChunk->layoutEntriesCount = layoutCount;
    meshChunk->vao.elementsCount = indexesCount /
        (indexPrimitive == LITE3D_PRIMITIVE_POINT ? 1 :
        (indexPrimitive == LITE3D_PRIMITIVE_LINE ? 2 : 3));

    lite3d_list_add_last_link(&meshChunk->node, &mesh->chunks);
    mesh->chunkCount++;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: 0x%x: chunk 0x%x: %s, cv/ov/sv %d/%db/%db, ci/oi %d/%db",
        mesh, meshChunk, indexPrimitive == LITE3D_PRIMITIVE_POINT ? "POINTS" : (indexPrimitive == LITE3D_PRIMITIVE_LINE ? "LINES" : "TRIANGLES"),
        meshChunk->vao.verticesCount, meshChunk->vao.verticesOffset, stride, meshChunk->vao.indexesCount, meshChunk->vao.indexesOffset);

    return meshChunk;
}

uint16_t lite3d_index_component_type_by_size(uint8_t size)
{
    return size == 1 ? GL_UNSIGNED_BYTE :
        (size == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
}

uint16_t lite3d_index_primitive_by_components(uint8_t count)
{
    return count == 1 ? GL_POINTS :
        (count == 2 ? GL_LINES : GL_TRIANGLES);
}