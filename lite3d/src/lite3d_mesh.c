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
#include <lite3d/lite3d_mesh.h>

static lite3d_vbo gAuxGlobalBuffer = {0, 0, 0};

int lite3d_mesh_init(struct lite3d_mesh *mesh)
{
    SDL_assert(mesh);

    memset(mesh, 0, sizeof (lite3d_mesh));
    lite3d_list_init(&mesh->chunks);
    
    if (lite3d_check_instanced_arrays())
    {
        if (!gAuxGlobalBuffer.vboID)
        {
            if (!lite3d_vbo_init(&gAuxGlobalBuffer))
                return LITE3D_FALSE;
            /*
            if (!lite3d_vbo_buffer(&gAuxGlobalBuffer, NULL, 20 * sizeof(kmMat4), LITE3D_VBO_DYNAMIC_DRAW))
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unable to setup auxiliary buffer",
                    LITE3D_CURRENT_FUNCTION);
                return LITE3D_FALSE;
            }*/
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

int lite3d_mesh_extend(struct lite3d_mesh *mesh, size_t verticesSize,
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
    lite3d_list_link_init(&meshChunk->node);
    meshChunk->hasIndexes = indexed;

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

lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_index(struct lite3d_mesh *mesh,
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

lite3d_mesh_chunk *lite3d_mesh_append_chunk(lite3d_mesh *mesh,
    const lite3d_mesh_layout *layout,
    size_t layoutCount,
    size_t stride,
    uint16_t componentType,
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

    if (!lite3d_mesh_chunk_init(meshChunk, LITE3D_TRUE))
    {
        lite3d_free_pooled(LITE3D_POOL_NO1, meshChunk);
        return NULL;
    }

    meshChunk->mesh = mesh;
    meshChunk->layout = (lite3d_mesh_layout *) lite3d_malloc(sizeof (lite3d_mesh_layout) * layoutCount);
    SDL_assert_release(meshChunk->layout);

    /* VAO set current */
    glBindVertexArray(meshChunk->vao.vaoID);
    /* use single VBO to store all data */
    glBindBuffer(mesh->vertexBuffer.role, mesh->vertexBuffer.vboID);
    /* bind all arrays and attribs into the current VAO */
    for (; i < layoutCount; ++i)
    {
        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex++, layout[i].count, GL_FLOAT,
            GL_FALSE, stride, LITE3D_BUFFER_OFFSET(vOffset));

        vOffset += layout[i].count * sizeof (GLfloat);
        meshChunk->layout[i] = layout[i];
    }

    // setup buffers for instancing rendering 
    if (mesh->auxBuffer)
    {
        glBindBuffer(mesh->vertexBuffer.role, mesh->auxBuffer->vboID);
        for(i = 0; i < 4; ++i)
        {
            glEnableVertexAttribArray(attribIndex);
            glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE, sizeof(kmMat4), LITE3D_BUFFER_OFFSET(i * sizeof(kmVec4)));
            glVertexAttribDivisor(attribIndex++, 1);
        }
    }

    if (indexesCount > 0 && indexesSize > 0)
    {
        glBindBuffer(mesh->indexBuffer.role, mesh->indexBuffer.vboID);
        meshChunk->hasIndexes = LITE3D_TRUE;
    }
    else
    {
        meshChunk->hasIndexes = LITE3D_FALSE;
    }

    /* end VAO binding */
    glBindVertexArray(0);

    meshChunk->vao.indexesOffset = indexesOffset;
    meshChunk->vao.indexType = componentType;
    meshChunk->vao.indexesCount = indexesCount;
    meshChunk->vao.indexesSize = indexesSize;
    meshChunk->vao.verticesCount = verticesCount;
    meshChunk->vao.verticesSize = verticesSize;
    meshChunk->vao.verticesOffset = verticesOffset;
    meshChunk->layoutEntriesCount = layoutCount;
    meshChunk->vao.elementsCount = (indexesCount > 0 ? indexesCount : verticesCount) / 3;

    memset(&meshChunk->boundingVol, 0, sizeof (meshChunk->boundingVol));

    lite3d_list_add_last_link(&meshChunk->node, &mesh->chunks);
    mesh->chunkCount++;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "MESH: %p: chunk %p: %s, cv/ov/sv %d/%ldb/%ldb, ci/oi %d/%ldb",
        (void *) mesh, (void *) meshChunk, "TRIANGLES",
        meshChunk->vao.verticesCount, meshChunk->vao.verticesOffset, stride, meshChunk->vao.indexesCount, meshChunk->vao.indexesOffset);

    return meshChunk;
}

uint16_t lite3d_index_component_type_by_size(uint8_t size)
{
    return size == sizeof(uint8_t) ? GL_UNSIGNED_BYTE :
        (size == sizeof(uint16_t) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
}

uint8_t lite3d_size_by_index_type(uint16_t type)
{
    return type == GL_UNSIGNED_BYTE ? sizeof(uint8_t) : 
        (type == GL_UNSIGNED_SHORT ? sizeof(uint16_t) : sizeof(uint32_t));
}