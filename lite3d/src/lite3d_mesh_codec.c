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
#include <SDL_assert.h>
#include <SDL_rwops.h>

#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_mesh_codec.h>

#define LITE3D_M_SIGNATURE          0xBEEB0001

#pragma pack(push, 1)

typedef struct lite3d_m_header
{
    uint32_t sig;
    int32_t version;
    int32_t chunkSectionSize;
    int32_t vertexSectionSize;
    int32_t indexSectionSize;
    int32_t chunkCount;
} lite3d_m_header;

typedef struct lite3d_m_chunk
{
    int32_t chunkSize;
    int32_t chunkLayoutCount;
    int32_t indexesCount;
    int32_t indexesSize;
    int32_t indexesOffset;
    int32_t verticesCount;
    int32_t verticesSize;
    int32_t verticesOffset;
    uint16_t elementType;
    uint16_t indexType;
} lite3d_m_chunk;

typedef struct lite3d_m_chunk_layout
{
    uint8_t binding;
    uint8_t count;
} lite3d_m_chunk_layout;

#pragma pack(pop)

size_t lite3d_indexed_mesh_m_encode_size(lite3d_indexed_mesh *mesh)
{
    size_t result = 0;
    lite3d_list_node *vaoLink;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    result += sizeof (lite3d_m_header);
    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        result += sizeof (lite3d_m_chunk);
        result += sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
    }

    result += mesh->vertexBuffer.size;
    result += mesh->indexBuffer.size;
    return result;
}

int lite3d_indexed_mesh_m_decode(lite3d_indexed_mesh *mesh,
    const void *buffer, size_t size, uint16_t access)
{
    SDL_RWops *stream;
    lite3d_m_header mheader;
    lite3d_m_chunk mchunk;
    lite3d_m_chunk_layout layout;
    lite3d_indexed_mesh_layout meshLayout[16];
    register int32_t i = 0;
    size_t indOffset = 0;
    size_t vertOffset = 0;
    void *mapped;
    SDL_assert(mesh);

    /* open memory stream */
    stream = SDL_RWFromConstMem(buffer, size);

    if (SDL_RWread(stream, &mheader, sizeof (mheader), 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (mheader.sig != LITE3D_M_SIGNATURE)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (mheader.version < LITE3D_VERSION_NUM)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (!lite3d_vbo_buffer(&mesh->vertexBuffer, NULL, mheader.vertexSectionSize,
        access))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (!lite3d_vbo_buffer(&mesh->indexBuffer, NULL, mheader.indexSectionSize,
        access))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    for (i = 0; i < mheader.chunkCount; ++i)
    {
        register int32_t j = 0;
        size_t stride = 0;
        if (SDL_RWread(stream, &mchunk, sizeof (mchunk), 1) != 1)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }

        if (mchunk.chunkLayoutCount > 16)
            mchunk.chunkLayoutCount = 16;

        for (; j < mchunk.chunkLayoutCount; ++j)
        {
            if (SDL_RWread(stream, &layout, sizeof (lite3d_m_chunk_layout), 1) != 1)
            {
                SDL_RWclose(stream);
                return LITE3D_FALSE;
            }

            stride += layout.count * sizeof (float);
            meshLayout[j].binding = layout.binding;
            meshLayout[j].count = layout.count;
        }

        /* append new batch */
        if (!lite3d_indexed_mesh_append_chunk(mesh, meshLayout, mchunk.chunkLayoutCount, stride,
            mchunk.indexType, mchunk.elementType, mchunk.indexesCount,
            mchunk.indexesSize, indOffset, mchunk.verticesCount, mchunk.verticesSize, vertOffset))
            return LITE3D_FALSE;

        indOffset += mchunk.indexesSize;
        vertOffset += mchunk.verticesSize;
        mesh->verticesCount += mchunk.verticesCount;
        mesh->elementsCount += mchunk.indexesCount / (mchunk.elementType == LITE3D_PRIMITIVE_POINT ? 1 : 
            (mchunk.elementType == LITE3D_PRIMITIVE_LINE ? 2 : 3));
    }

    if (SDL_RWseek(stream, sizeof (mheader) + mheader.chunkSectionSize, RW_SEEK_SET) < 0)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if ((mapped = lite3d_vbo_map(&mesh->vertexBuffer, LITE3D_VBO_MAP_WRITE_ONLY)) == NULL)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* read vertex section in mapped vertex buffer directly */
    if (SDL_RWread(stream, mapped, mheader.vertexSectionSize, 1) != 1)
    {
        SDL_RWclose(stream);
        lite3d_vbo_unmap(&mesh->vertexBuffer);
        return LITE3D_FALSE;
    }

    lite3d_vbo_unmap(&mesh->vertexBuffer);

    if ((mapped = lite3d_vbo_map(&mesh->indexBuffer, LITE3D_VBO_MAP_WRITE_ONLY)) == NULL)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* read index section in mapped index buffer directly */
    if (SDL_RWread(stream, mapped, mheader.indexSectionSize, 1) != 1)
    {
        SDL_RWclose(stream);
        lite3d_vbo_unmap(&mesh->indexBuffer);
        return LITE3D_FALSE;
    }

    lite3d_vbo_unmap(&mesh->indexBuffer);
    SDL_RWclose(stream);
    return LITE3D_TRUE;
}

int lite3d_indexed_mesh_m_encode(lite3d_indexed_mesh *mesh,
    void *buffer, size_t size)
{
    lite3d_list_node *vaoLink;
    lite3d_mesh_chunk *meshChunk;
    lite3d_m_header mheader;
    lite3d_m_chunk mchunk;
    lite3d_m_chunk_layout layout;
    SDL_RWops *stream;
    void *vboData;
    SDL_assert(mesh);


    mheader.sig = LITE3D_M_SIGNATURE;
    mheader.version = LITE3D_VERSION_NUM;
    mheader.vertexSectionSize = mesh->vertexBuffer.size;
    mheader.indexSectionSize = mesh->indexBuffer.size;
    mheader.chunkCount = mesh->chunkCount;
    mheader.chunkSectionSize = 0;


    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        mheader.chunkSectionSize += sizeof (lite3d_m_chunk);
        mheader.chunkSectionSize += sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
    }

    /* open memory stream */
    stream = SDL_RWFromMem(buffer, size);
    /* write header */
    if (SDL_RWwrite(stream, &mheader, sizeof (mheader), 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write chunks data */
    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        int i = 0;
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        mchunk.chunkSize = sizeof (lite3d_m_chunk) +
            sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
        mchunk.chunkLayoutCount = meshChunk->layoutEntriesCount;
        mchunk.indexesCount = meshChunk->vao.indexesCount;
        mchunk.indexesSize = meshChunk->vao.indexesSize;
        mchunk.indexesOffset = meshChunk->vao.indexesOffset;
        mchunk.verticesCount = meshChunk->vao.verticesCount;
        mchunk.verticesSize = meshChunk->vao.verticesSize;
        mchunk.verticesOffset = meshChunk->vao.verticesOffset;
        mchunk.elementType = meshChunk->vao.elementType;
        mchunk.indexType = meshChunk->vao.indexType;

        if (SDL_RWwrite(stream, &mchunk, sizeof (mchunk), 1) != 1)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }

        for (; i < mchunk.chunkLayoutCount; ++i)
        {
            layout.binding = meshChunk->layout[i].binding;
            layout.count = meshChunk->layout[i].count;

            if (SDL_RWwrite(stream, &layout, sizeof (layout), 1) != 1)
            {
                SDL_RWclose(stream);
                return LITE3D_FALSE;
            }
        }
    }


    /* map vertex buffer */
    if ((vboData = lite3d_vbo_map(&mesh->vertexBuffer, LITE3D_VBO_MAP_READ_ONLY)) == NULL)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write vertex data */
    if (SDL_RWwrite(stream, vboData, mesh->vertexBuffer.size, 1) != 1)
    {
        SDL_RWclose(stream);
        lite3d_vbo_unmap(&mesh->vertexBuffer);
        return LITE3D_FALSE;
    }
    lite3d_vbo_unmap(&mesh->vertexBuffer);

    /* map index buffer */
    if ((vboData = lite3d_vbo_map(&mesh->indexBuffer, LITE3D_VBO_MAP_READ_ONLY)) == NULL)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write index data */
    if (SDL_RWwrite(stream, vboData, mesh->indexBuffer.size, 1) != 1)
    {
        SDL_RWclose(stream);
        lite3d_vbo_unmap(&mesh->indexBuffer);
        return LITE3D_FALSE;
    }
    lite3d_vbo_unmap(&mesh->indexBuffer);

    SDL_RWclose(stream);
    return LITE3D_TRUE;
}