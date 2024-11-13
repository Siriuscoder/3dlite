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
#include "lite3d_common_test.h"
#include <functional>

#include <lite3d/lite3d_mesh_codec.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

#ifdef INCLUDE_ASSIMP

class VBO_Test : public Lite3dCommon
{
public:

    static size_t makeHash(void *b, size_t len)
    {
        std::string_view v(static_cast<const char *>(b), len);
        return std::hash<std::string_view>{}(v);
    }

    static int meshLoadingTest(void *userdata)
    {
        lite3d_mesh mVBO;
        lite3d_pack *fileSysPack = lite3d_pack_open("tests/", 0, 1000000);
        EXPECT_TRUE(fileSysPack != NULL);
        lite3d_file *meshFile = lite3d_pack_file_load(fileSysPack, "pack/minigun/minigun.3ds");
        EXPECT_TRUE(lite3d_mesh_init(&mVBO, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_assimp_mesh_load(&mVBO, meshFile, NULL, 0) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_list_count(&mVBO.chunks) == 1);
        EXPECT_EQ(mVBO.verticesCount, 17865u);
        EXPECT_EQ(mVBO.elementsCount, 5955u);
    
        lite3d_mesh_purge(&mVBO);
        lite3d_pack_close(fileSysPack);
        /* quit immediatly */
        return LITE3D_FALSE;
    }

    static int bufferMapTest(void *userdata)
    {
        lite3d_mesh mVBO;
        lite3d_pack *fileSysPack = lite3d_pack_open("tests/", 0, 1000000);
        EXPECT_TRUE(fileSysPack != NULL);
        lite3d_file *meshFile = lite3d_pack_file_load(fileSysPack, "pack/minigun/minigun.3ds");
        EXPECT_TRUE(lite3d_mesh_init(&mVBO, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_assimp_mesh_load(&mVBO, meshFile, NULL, 0) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_list_count(&mVBO.chunks) == 1);

        char *pVertexBufferOrig = (char *)lite3d_malloc(mVBO.vertexBuffer.size);
        char *pIndexBufferOrig = (char *)lite3d_malloc(mVBO.vertexBuffer.size);
        EXPECT_TRUE(lite3d_vbo_get_buffer(&mVBO.vertexBuffer, pVertexBufferOrig, 0, mVBO.vertexBuffer.size) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_vbo_get_buffer(&mVBO.indexBuffer, pIndexBufferOrig, 0, mVBO.indexBuffer.size) == LITE3D_TRUE);

        void *pVertexBuffer = lite3d_vbo_map(&mVBO.vertexBuffer, LITE3D_VBO_MAP_READ_ONLY);
        void *pIndexBuffer = lite3d_vbo_map(&mVBO.indexBuffer, LITE3D_VBO_MAP_READ_ONLY);

        EXPECT_TRUE(pVertexBuffer);
        EXPECT_TRUE(pIndexBuffer);

        if (pVertexBuffer)
        {
            EXPECT_TRUE(memcmp(pVertexBuffer, pVertexBufferOrig, mVBO.vertexBuffer.size) == 0);
        }

        if (pIndexBuffer)
        {
            EXPECT_TRUE(memcmp(pIndexBuffer, pIndexBufferOrig, mVBO.indexBuffer.size) == 0);
        }
    
        lite3d_vbo_unmap(&mVBO.vertexBuffer);
        lite3d_vbo_unmap(&mVBO.indexBuffer);
        lite3d_mesh_purge(&mVBO);
        lite3d_pack_close(fileSysPack);
        lite3d_free(pVertexBufferOrig);
        lite3d_free(pIndexBufferOrig);
        /* quit immediatly */
        return LITE3D_FALSE;
    }
    
    static int encodeDecode_M_formatTest(void *userdata)
    {
        lite3d_mesh mVBO;
        lite3d_mesh mVBO1;
    
        lite3d_pack *fileSysPack = lite3d_pack_open("tests/", 0, 1000000);
        EXPECT_TRUE(fileSysPack != NULL);
        lite3d_file *meshFile = lite3d_pack_file_load(fileSysPack, "pack/minigun/minigun.3ds");
        EXPECT_TRUE(lite3d_mesh_init(&mVBO, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_assimp_mesh_load(&mVBO, meshFile, NULL,
            LITE3D_OPTIMIZE_MESH_FLAG) == LITE3D_TRUE);
    
        size_t mfileSize = lite3d_mesh_m_encode_size(&mVBO);
        void *encodeBuffer = lite3d_malloc(mfileSize);
        EXPECT_TRUE(lite3d_mesh_m_encode(&mVBO, encodeBuffer, mfileSize) == LITE3D_TRUE);
    
        EXPECT_TRUE(lite3d_mesh_init(&mVBO1, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_mesh_m_decode(&mVBO1, encodeBuffer, mfileSize) == LITE3D_TRUE);
    
        EXPECT_EQ(lite3d_list_count(&mVBO.chunks), lite3d_list_count(&mVBO1.chunks));
        EXPECT_EQ(mVBO.verticesCount, mVBO1.verticesCount);
        EXPECT_EQ(mVBO.elementsCount, mVBO1.elementsCount);
    
        lite3d_free(encodeBuffer);
        lite3d_mesh_purge(&mVBO);
        lite3d_mesh_purge(&mVBO1);
        lite3d_pack_close(fileSysPack);
        /* quit immediatly */
        return LITE3D_FALSE;
    }

/*
    static int encodeDecode_ConvertIndexTest(void *userdata)
    {
        lite3d_mesh mVBO;
        lite3d_mesh mVBO1;
    
        lite3d_pack *fileSysPack = lite3d_pack_open("samples/", 0, 1000000);
        EXPECT_TRUE(fileSysPack != NULL);
        lite3d_file *meshFile = lite3d_pack_file_load(fileSysPack, "models/meshes/minigun.m");
        EXPECT_TRUE(lite3d_mesh_init(&mVBO) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_mesh_init(&mVBO1) == LITE3D_TRUE);
        EXPECT_TRUE(lite3d_mesh_m_decode(&mVBO, meshFile->fileBuff, meshFile->fileSize, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);

        for(size_t j = 0; j < mVBO.chunks.size; ++j)
        {
            auto chunk = static_cast<lite3d_mesh_chunk *>(lite3d_array_get(&mVBO.chunks, j));
            uint32_t *newIndex = new uint32_t[chunk->vao.indexesCount];

            if (chunk->vao.indexElemSize == 1)
            {
                uint8_t *iptr = (uint8_t *)lite3d_vbo_map(&mVBO.indexBuffer, LITE3D_VBO_MAP_READ_ONLY) + chunk->vao.indexesOffset;
                for (uint32_t i = 0; i < chunk->vao.indexesCount; ++i)
                {
                    newIndex[i] = iptr[i];
                }

                lite3d_vbo_unmap(&mVBO.indexBuffer);
            }
            else if (chunk->vao.indexElemSize == 2)
            {
                uint16_t *iptr = (uint16_t *)((uint8_t *)lite3d_vbo_map(&mVBO.indexBuffer, LITE3D_VBO_MAP_READ_ONLY) + chunk->vao.indexesOffset);
                for (uint32_t i = 0; i < chunk->vao.indexesCount; ++i)
                {
                    newIndex[i] = iptr[i];
                }

                lite3d_vbo_unmap(&mVBO.indexBuffer);
            }
            else if (chunk->vao.indexElemSize == 4)
            {
                uint32_t *iptr = (uint32_t *)((uint8_t *)lite3d_vbo_map(&mVBO.indexBuffer, LITE3D_VBO_MAP_READ_ONLY) + chunk->vao.indexesOffset);
                for (uint32_t i = 0; i < chunk->vao.indexesCount; ++i)
                {
                    newIndex[i] = iptr[i];
                }

                lite3d_vbo_unmap(&mVBO.indexBuffer);
            }

            uint8_t *vptr = (uint8_t *)lite3d_vbo_map(&mVBO.vertexBuffer, LITE3D_VBO_MAP_READ_ONLY) + chunk->vao.verticesOffset;
            EXPECT_TRUE(lite3d_mesh_indexed_extend_from_memory(&mVBO1, vptr, chunk->vao.verticesCount, chunk->layout, chunk->layoutEntriesCount, 
                newIndex, chunk->vao.elementsCount, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
            
            static_cast<lite3d_mesh_chunk *>(lite3d_array_get(&mVBO1.chunks, j))->materialIndex = chunk->materialIndex;
            static_cast<lite3d_mesh_chunk *>(lite3d_array_get(&mVBO1.chunks, j))->boundingVol = chunk->boundingVol;
            
            lite3d_vbo_unmap(&mVBO.vertexBuffer);
            delete [] newIndex;
        }

        size_t mfileSize = lite3d_mesh_m_encode_size(&mVBO1);
        void *encodeBuffer = lite3d_malloc(mfileSize);
        EXPECT_TRUE(lite3d_mesh_m_encode(&mVBO1, encodeBuffer, mfileSize) == LITE3D_TRUE);
    
        auto f = fopen("minigun.m", "wb");
        fwrite(encodeBuffer, mfileSize, 1, f);
        fclose(f);
    
        lite3d_free(encodeBuffer);
        lite3d_mesh_purge(&mVBO);
        lite3d_mesh_purge(&mVBO1);
        lite3d_pack_close(fileSysPack);
        return LITE3D_FALSE;
    }
*/
};

LITE3D_GTEST_DECLARE(VBO_Test, meshLoading, meshLoadingTest)
LITE3D_GTEST_DECLARE(VBO_Test, mapping, bufferMapTest)
LITE3D_GTEST_DECLARE(VBO_Test, encodeDecode_M_format, encodeDecode_M_formatTest)

#endif
