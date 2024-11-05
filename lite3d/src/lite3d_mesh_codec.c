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

#include <SDL_assert.h>
#include <SDL_rwops.h>
#include <SDL_log.h>

#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_mesh_codec.h>

#define LITE3D_M_SIGNATURE          0xBEEB0001
#define CHUNK_LAYOUT_MAX_COUNT      32

#pragma pack(push, 1)

typedef struct lite3d_m_header
{
    uint32_t sig;
    int32_t version;
    uint32_t chunkSectionSize;
    uint32_t vertexSectionSize;
    uint32_t indexSectionSize;
    uint32_t chunkCount;
} lite3d_m_header;

typedef struct lite3d_m_chunk
{
    uint32_t chunkSize;
    uint32_t chunkLayoutCount;
    uint32_t indexesCount;
    uint32_t indexesSize;
    uint32_t indexesOffset;
    uint32_t verticesCount;
    uint32_t verticesSize;
    uint32_t verticesOffset;
    uint8_t indexElemSize; // it is not used anymore, always 4 bytes per index 
    uint32_t materialIndex;
    lite3d_bounding_vol boundingVol;
} lite3d_m_chunk;

typedef struct lite3d_m_chunk_layout
{
    uint8_t binding;
    uint8_t count;
} lite3d_m_chunk_layout;

#pragma pack(pop)

static int lite3d_realloc_buffer(lite3d_vbo *buffer, size_t bufferOffset, size_t size)
{
    SDL_assert(buffer);
    // Не вмещаемся, требуется реалокация буфера
    if (bufferOffset + size > buffer->size)
    {
        if (!lite3d_vbo_extend(buffer, bufferOffset + size - buffer->size, buffer->access))
        {
            return LITE3D_FALSE;
        }
    }

    return LITE3D_TRUE;
}

static int lite3d_write_buffer_to_stream(lite3d_vbo *buffer, SDL_RWops *stream)
{
    void *vboData;
    
    SDL_assert(buffer);
    SDL_assert(stream);

    if (lite3d_check_map_buffer())
    {
        if ((vboData = lite3d_vbo_map(buffer, LITE3D_VBO_MAP_READ_ONLY)) == NULL)
        {
            return LITE3D_FALSE;
        }

        if (SDL_RWwrite(stream, vboData, buffer->size, 1) != 1)
        {
            lite3d_vbo_unmap(buffer);
            return LITE3D_FALSE;
        }
        
        lite3d_vbo_unmap(buffer);
    }
    else
    {
        vboData = lite3d_malloc(buffer->size);
        if (!vboData)
        {
            return LITE3D_FALSE;
        }

        if (!lite3d_vbo_get_buffer(buffer, vboData, 0, buffer->size))
        {
            lite3d_free(vboData);
            return LITE3D_FALSE;
        }

        if (SDL_RWwrite(stream, vboData, buffer->size, 1) != 1)
        {
            lite3d_free(vboData);
            return LITE3D_FALSE;
        }

        lite3d_free(vboData);
    }

    return LITE3D_TRUE;
}

static int lite3d_append_buffer_from_stream(lite3d_vbo *buffer, size_t bufferOffset, size_t size, SDL_RWops *stream)
{
    SDL_assert(buffer);
    SDL_assert(stream);

    if (lite3d_check_map_buffer())
    {
        uint8_t *mapped;
        if ((mapped = lite3d_vbo_map(buffer, LITE3D_VBO_MAP_WRITE_ONLY)) == NULL)
        {
            return LITE3D_FALSE;
        }

        /* read vertex section in mapped vertex buffer directly */
        if (SDL_RWread(stream, mapped + bufferOffset, size, 1) != 1)
        {
            lite3d_vbo_unmap(buffer);
            return LITE3D_FALSE;
        }

        lite3d_vbo_unmap(buffer);
    }
    else
    {
        void *tmpBuf = lite3d_malloc(size);
        if (!tmpBuf)
        {
            return LITE3D_FALSE;
        }

        /* read vertex section in mapped vertex buffer directly */
        if (SDL_RWread(stream, tmpBuf, size, 1) != 1)
        {
            lite3d_free(tmpBuf);
            return LITE3D_FALSE;
        }

        if (!lite3d_vbo_subbuffer(buffer, tmpBuf, bufferOffset, size))
        {
            lite3d_free(tmpBuf);
            return LITE3D_FALSE;
        }

        lite3d_free(tmpBuf);
    }

    return LITE3D_TRUE;
}

size_t lite3d_mesh_m_encode_size(lite3d_mesh *mesh)
{
    size_t result = 0;
    lite3d_list_node *link;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    result += sizeof (lite3d_m_header);

    for (link = mesh->chunks.l.next; link != &mesh->chunks.l; link = lite3d_list_next(link))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);
        result += sizeof (lite3d_m_chunk);
        result += sizeof (lite3d_m_chunk_layout) * meshChunk->layout.size;
    }

    result += mesh->vertexBuffer.size;
    result += mesh->indexBuffer.size;
    return result;
}

int lite3d_mesh_m_decode(lite3d_mesh *mesh,
    const void *buffer, size_t size, uint16_t access)
{
    SDL_RWops *stream;
    lite3d_m_header mheader;
    lite3d_m_chunk mchunk;
    lite3d_m_chunk_layout layout;
    lite3d_vao_layout meshLayout[CHUNK_LAYOUT_MAX_COUNT];
    register uint32_t i = 0;
    size_t indicesOffset = 0, initialIndicesOffset = 0;
    size_t verticesOffset = 0, initialVerticesOffset = 0;
    uint32_t chunkSectionOffset = 0;
    lite3d_mesh_chunk *thisChunk = NULL;
    
    SDL_assert(mesh);
    SDL_assert(buffer);

    // Если mesh уже содержит данные то надо корректно вычислить offset от последнего чанка
    if (!lite3d_list_is_empty(&mesh->chunks))
    {
        lite3d_mesh_chunk *lastChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, lite3d_list_last_link(&mesh->chunks), link);
        initialIndicesOffset = indicesOffset = lastChunk->vao.indexesOffset + lastChunk->vao.indexesSize;
        initialVerticesOffset = verticesOffset = lastChunk->vao.verticesOffset + lastChunk->vao.verticesSize;
    }

    /* open memory stream */
    stream = SDL_RWFromConstMem(buffer, (int)size);

    if (SDL_RWread(stream, &mheader, sizeof (mheader), 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (mheader.sig != LITE3D_M_SIGNATURE)
    {
        SDL_RWclose(stream);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Signature mismatch: %d vs %d",
            LITE3D_CURRENT_FUNCTION, mheader.sig, LITE3D_M_SIGNATURE);
        return LITE3D_FALSE;
    }

    if (!lite3d_realloc_buffer(&mesh->vertexBuffer, initialVerticesOffset, mheader.vertexSectionSize))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (!lite3d_realloc_buffer(&mesh->indexBuffer, initialIndicesOffset, mheader.indexSectionSize))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }
    
    mesh->version = mheader.version;
    for (i = 0; i < mheader.chunkCount; ++i)
    {
        register uint32_t j = 0;
        uint32_t stride = 0;
        
        if (SDL_RWseek(stream, sizeof (mheader) + chunkSectionOffset, RW_SEEK_SET) < 0)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }
        
        if (SDL_RWread(stream, &mchunk, sizeof (mchunk), 1) != 1)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }

        if (mchunk.chunkLayoutCount > CHUNK_LAYOUT_MAX_COUNT)
            mchunk.chunkLayoutCount = CHUNK_LAYOUT_MAX_COUNT;

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
        if (!(thisChunk = lite3d_mesh_append_chunk(mesh, meshLayout, mchunk.chunkLayoutCount, stride,
            mchunk.indexesCount, mchunk.indexesSize, indicesOffset, mchunk.verticesCount, mchunk.verticesSize, verticesOffset)))
        {
            return LITE3D_FALSE;
        }

        /* set material index to currently added meshChunk */
        thisChunk->materialIndex = mchunk.materialIndex;
        thisChunk->boundingVol = mchunk.boundingVol;

        indicesOffset += mchunk.indexesSize;
        verticesOffset += mchunk.verticesSize;
        mesh->verticesCount += mchunk.verticesCount;
        mesh->elementsCount += mchunk.indexesCount / 3;
        
        chunkSectionOffset += mchunk.chunkSize;
    }

    if (SDL_RWseek(stream, sizeof (mheader) + mheader.chunkSectionSize, RW_SEEK_SET) < 0)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (mheader.vertexSectionSize > 0)
    {
        if(!lite3d_append_buffer_from_stream(&mesh->vertexBuffer, initialVerticesOffset, 
            mheader.vertexSectionSize, stream))
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }
    }
    else
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: Vertices section has a zero size",
            LITE3D_CURRENT_FUNCTION);
    }

    if (mheader.indexSectionSize > 0)
    {
        if(!lite3d_append_buffer_from_stream(&mesh->indexBuffer, initialIndicesOffset, 
            mheader.indexSectionSize, stream))
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }
    }

    SDL_RWclose(stream);
    return LITE3D_TRUE;
}

int lite3d_mesh_m_encode(lite3d_mesh *mesh,
    void *buffer, size_t size)
{
    lite3d_list_node *link;
    lite3d_mesh_chunk *meshChunk;
    lite3d_m_header mheader;
    lite3d_m_chunk mchunk;
    lite3d_m_chunk_layout layout;
    SDL_RWops *stream;

    SDL_assert(mesh);
    SDL_assert(buffer);

    mheader.sig = LITE3D_M_SIGNATURE;
    mheader.version = LITE3D_VERSION_NUM;
    mheader.vertexSectionSize = (uint32_t)mesh->vertexBuffer.size;
    mheader.indexSectionSize = (uint32_t)mesh->indexBuffer.size;
    mheader.chunkCount = (uint32_t)lite3d_list_count(&mesh->chunks);
    mheader.chunkSectionSize = 0;

    for (link = mesh->chunks.l.next; link != &mesh->chunks.l; link = lite3d_list_next(link))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);
        mheader.chunkSectionSize += (uint32_t)(sizeof (lite3d_m_chunk));
        mheader.chunkSectionSize += (uint32_t)(sizeof (lite3d_m_chunk_layout) * meshChunk->layout.size);
    }

    /* open memory stream */
    stream = SDL_RWFromMem(buffer, (int)size);
    /* write header */
    if (SDL_RWwrite(stream, &mheader, sizeof (mheader), 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write chunks data */
    for (link = mesh->chunks.l.next; link != &mesh->chunks.l; link = lite3d_list_next(link))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, link, link);

        memset(&mchunk, 0, sizeof(mchunk));
        mchunk.chunkSize = (uint32_t)(sizeof (lite3d_m_chunk) +
            sizeof (lite3d_m_chunk_layout) * meshChunk->layout.size);
        mchunk.chunkLayoutCount = (uint32_t)(meshChunk->layout.size);
        mchunk.indexesCount = meshChunk->vao.indexesCount;
        mchunk.indexesSize = (uint32_t)meshChunk->vao.indexesSize;
        mchunk.indexesOffset = (uint32_t)meshChunk->vao.indexesOffset;
        mchunk.verticesCount = meshChunk->vao.verticesCount;
        mchunk.verticesSize = (uint32_t)meshChunk->vao.verticesSize;
        mchunk.verticesOffset = (uint32_t)meshChunk->vao.verticesOffset;
        mchunk.indexElemSize = sizeof(uint32_t);
        mchunk.materialIndex = meshChunk->materialIndex;
        mchunk.boundingVol = meshChunk->boundingVol;

        if (SDL_RWwrite(stream, &mchunk, sizeof (mchunk), 1) != 1)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }

        for (uint32_t i = 0; i < mchunk.chunkLayoutCount; ++i)
        {
            lite3d_vao_layout *playout = (lite3d_vao_layout *)lite3d_array_get(&meshChunk->layout, i);
            layout.binding = playout->binding;
            layout.count = playout->count;

            if (SDL_RWwrite(stream, &layout, sizeof (layout), 1) != 1)
            {
                SDL_RWclose(stream);
                return LITE3D_FALSE;
            }
        }
    }

    if (!lite3d_write_buffer_to_stream(&mesh->vertexBuffer, stream))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if (mesh->indexBuffer.size > 0)
    {
        if (!lite3d_write_buffer_to_stream(&mesh->indexBuffer, stream))
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }
    }

    SDL_RWclose(stream);
    return LITE3D_TRUE;
}
