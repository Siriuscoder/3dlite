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
#include <SDL_assert.h>
#include <SDL_rwops.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_m_codec.h>

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
    int32_t elementsCount;
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

    result += sizeof(lite3d_m_header);
    for (vaoLink = mesh->chunks.l.next;
        vaoLink != &mesh->chunks.l; vaoLink = lite3d_list_next(vaoLink))
    {
        meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, vaoLink, node);
        result += sizeof(lite3d_m_chunk);
        result += sizeof(lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
    }

    result += mesh->vertexBuffer.size;
    result += mesh->indexBuffer.size;
    return result;
}

int lite3d_indexed_mesh_m_decode(lite3d_indexed_mesh *mesh, 
    void *data, size_t size, uint16_t access)
{



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
        mheader.chunkSectionSize += sizeof(lite3d_m_chunk);
        mheader.chunkSectionSize += sizeof(lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
    }

    /* open memory stream */
    stream = SDL_RWFromMem(buffer, size);
    /* write header */
    if(SDL_RWwrite(stream, &mheader, sizeof(mheader), 1) != 1)
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
        mchunk.chunkSize = sizeof(lite3d_m_chunk) + 
            sizeof(lite3d_m_chunk_layout) * meshChunk->layoutEntriesCount;
        mchunk.chunkLayoutCount = meshChunk->layoutEntriesCount;
        mchunk.elementsCount = meshChunk->vao.elementsCount;
        mchunk.indexesCount = meshChunk->vao.indexesCount;
        mchunk.indexesSize = meshChunk->vao.indexesSize;
        mchunk.indexesOffset = meshChunk->vao.indexesOffset;
        mchunk.verticesCount = meshChunk->vao.verticesCount;
        mchunk.verticesSize = meshChunk->vao.verticesSize;
        mchunk.verticesOffset = meshChunk->vao.verticesOffset;
        mchunk.elementType = meshChunk->vao.elementType;
        mchunk.indexType = meshChunk->vao.indexType;

        if(SDL_RWwrite(stream, &mchunk, sizeof(mchunk), 1) != 1)
        {
            SDL_RWclose(stream);
            return LITE3D_FALSE;
        }

        for(; i < mchunk.chunkLayoutCount; ++i)
        {
            layout.binding = meshChunk->layout[i].binding;
            layout.count = meshChunk->layout[i].count;

            if(SDL_RWwrite(stream, &layout, sizeof(layout), 1) != 1)
            {
                SDL_RWclose(stream);
                return LITE3D_FALSE;
            }
        }
    }

    lite3d_misc_gl_error_stack_clean();
    /* map vertex buffer */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer.vboID);
    vboData = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    if (lite3d_misc_check_gl_error())
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write vertex data */
    if(SDL_RWwrite(stream, vboData, mesh->vertexBuffer.size, 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    /* map index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer.vboID);
    vboData = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);

    if (lite3d_misc_check_gl_error())
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }

    /* write index data */
    if(SDL_RWwrite(stream, vboData, mesh->indexBuffer.size, 1) != 1)
    {
        SDL_RWclose(stream);
        return LITE3D_FALSE;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SDL_RWclose(stream);
    return LITE3D_TRUE;
}