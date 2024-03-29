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
    uint8_t indexElemSize;
    uint32_t materialIndex;
    lite3d_bounding_vol boundingVol;
} lite3d_m_chunk;

typedef struct lite3d_m_chunk_layout
{
    uint8_t binding;
    uint8_t count;
} lite3d_m_chunk_layout;

#pragma pack(pop)

static int _load_mesh_from_stream(lite3d_mesh *mesh, SDL_RWops *stream)
{
    lite3d_misc_gl_error_stack_clean();
    if (lite3d_check_map_buffer())
    {
        void *mapped;
        if ((mapped = lite3d_vbo_map(&mesh->vertexBuffer, LITE3D_VBO_MAP_WRITE_ONLY)) == NULL)
        {
            return LITE3D_FALSE;
        }

        /* read vertex section in mapped vertex buffer directly */
        if (SDL_RWread(stream, mapped, mesh->vertexBuffer.size, 1) != 1)
        {
            lite3d_vbo_unmap(&mesh->vertexBuffer);
            return LITE3D_FALSE;
        }

        lite3d_vbo_unmap(&mesh->vertexBuffer);

        if ((mapped = lite3d_vbo_map(&mesh->indexBuffer, LITE3D_VBO_MAP_WRITE_ONLY)) == NULL)
        {
            return LITE3D_FALSE;
        }

        /* read index section in mapped index buffer directly */
        if (SDL_RWread(stream, mapped, mesh->indexBuffer.size, 1) != 1)
        {
            lite3d_vbo_unmap(&mesh->indexBuffer);
            return LITE3D_FALSE;
        }

        lite3d_vbo_unmap(&mesh->indexBuffer);
    }
    else
    {
        void *tbuffer = lite3d_malloc(mesh->vertexBuffer.size);
        /* read vertex section in mapped vertex buffer directly */
        if (SDL_RWread(stream, tbuffer, mesh->vertexBuffer.size, 1) != 1)
        {
            lite3d_free(tbuffer);
            return LITE3D_FALSE;
        }

        lite3d_vbo_subbuffer(&mesh->vertexBuffer, tbuffer, 0, mesh->vertexBuffer.size);
        lite3d_free(tbuffer);

        tbuffer = lite3d_malloc(mesh->indexBuffer.size);
        /* read vertex section in mapped vertex buffer directly */
        if (SDL_RWread(stream, tbuffer, mesh->indexBuffer.size, 1) != 1)
        {
            lite3d_free(tbuffer);
            return LITE3D_FALSE;
        }

        lite3d_vbo_subbuffer(&mesh->indexBuffer, tbuffer, 0, mesh->indexBuffer.size);
        lite3d_free(tbuffer);
    }

    return !LITE3D_CHECK_GL_ERROR;
}

size_t lite3d_mesh_m_encode_size(lite3d_mesh *mesh)
{
    size_t result = 0;
    lite3d_mesh_chunk *meshChunk;
    SDL_assert(mesh);

    result += sizeof (lite3d_m_header);

    LITE3D_ARR_FOREACH(&mesh->chunks, lite3d_mesh_chunk, meshChunk)
    {
        result += sizeof (lite3d_m_chunk);
        result += sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
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
    register int32_t i = 0;
    size_t indOffset = 0;
    size_t vertOffset = 0;
    uint32_t chunkSectionOffset = 0;
    lite3d_mesh_chunk *thisChunk;
    
    SDL_assert(mesh);

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
    
    mesh->version = mheader.version;

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
            lite3d_index_component_type_by_size(mchunk.indexElemSize), mchunk.indexesCount,
            mchunk.indexesSize, indOffset, mchunk.verticesCount, mchunk.verticesSize, vertOffset)))
        {
            return LITE3D_FALSE;
        }

        /* set material index to currently added meshChunk */
        thisChunk->materialIndex = mchunk.materialIndex;
        thisChunk->boundingVol = mchunk.boundingVol;

        indOffset += mchunk.indexesSize;
        vertOffset += mchunk.verticesSize;
        mesh->verticesCount += mchunk.verticesCount;
        mesh->elementsCount += mchunk.indexesCount / 3;
        
        chunkSectionOffset += mchunk.chunkSize;
    }

    if (SDL_RWseek(stream, sizeof (mheader) + mheader.chunkSectionSize, RW_SEEK_SET) < 0)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    if(!_load_mesh_from_stream(mesh, stream))
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    SDL_RWclose(stream);
    return LITE3D_TRUE;
}

int lite3d_mesh_m_encode(lite3d_mesh *mesh,
    void *buffer, size_t size)
{
    lite3d_mesh_chunk *meshChunk;
    lite3d_m_header mheader;
    lite3d_m_chunk mchunk;
    lite3d_m_chunk_layout layout;
    SDL_RWops *stream;
    void *vboData;
    SDL_assert(mesh);


    mheader.sig = LITE3D_M_SIGNATURE;
    mheader.version = LITE3D_VERSION_NUM;
    mheader.vertexSectionSize = (int32_t)mesh->vertexBuffer.size;
    mheader.indexSectionSize = (int32_t)mesh->indexBuffer.size;
    mheader.chunkCount = (int32_t)mesh->chunks.size;
    mheader.chunkSectionSize = 0;


    LITE3D_ARR_FOREACH(&mesh->chunks, lite3d_mesh_chunk, meshChunk)
    {
        mheader.chunkSectionSize += sizeof (lite3d_m_chunk);
        mheader.chunkSectionSize += sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
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
    LITE3D_ARR_FOREACH(&mesh->chunks, lite3d_mesh_chunk, meshChunk)
    {
        int i = 0;

        memset(&mchunk, 0, sizeof(mchunk));
        mchunk.chunkSize = sizeof (lite3d_m_chunk) +
            sizeof (lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
        mchunk.chunkLayoutCount = meshChunk->layoutEntriesCount;
        mchunk.indexesCount = meshChunk->vao.indexesCount;
        mchunk.indexesSize = (int32_t)meshChunk->vao.indexesSize;
        mchunk.indexesOffset = (int32_t)meshChunk->vao.indexesOffset;
        mchunk.verticesCount = meshChunk->vao.verticesCount;
        mchunk.verticesSize = (int32_t)meshChunk->vao.verticesSize;
        mchunk.verticesOffset = (int32_t)meshChunk->vao.verticesOffset;
        mchunk.indexElemSize = lite3d_size_by_index_type(meshChunk->vao.indexType);
        mchunk.materialIndex = meshChunk->materialIndex;
        mchunk.boundingVol = meshChunk->boundingVol;

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
