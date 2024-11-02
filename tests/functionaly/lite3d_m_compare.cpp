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
#include <SDL_assert.h>
#include <gtest/gtest.h>

#include <lite3d/lite3d_mesh_codec.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>
#include "lite3d_common_test.h"

#ifdef INCLUDE_ASSIMP

class MeshCompare_Test : public Lite3dCommon
{
public:


    static void loadMeshes(lite3d_mesh *ubrMesh, lite3d_mesh *brMesh, lite3d_pack **fileSysPack)
    {
        *fileSysPack = lite3d_pack_open("tests/", 0, 1000000);
        SDL_assert_release(*fileSysPack != NULL);

        lite3d_file *ubrMeshFile = lite3d_pack_file_load(*fileSysPack, "meshes/VURmCorner_ubr.m");
        SDL_assert_release(ubrMeshFile != NULL);
        lite3d_file *brMeshFile = lite3d_pack_file_load(*fileSysPack, "meshes/VURmCorner_br.m");
        SDL_assert_release(brMeshFile != NULL);

        SDL_assert_release(lite3d_mesh_init(ubrMesh) == LITE3D_TRUE);
        SDL_assert_release(lite3d_mesh_init(brMesh) == LITE3D_TRUE);

        SDL_assert_release(lite3d_mesh_m_decode(ubrMesh, ubrMeshFile->fileBuff, ubrMeshFile->fileSize, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
        SDL_assert_release(lite3d_mesh_m_decode(brMesh, brMeshFile->fileBuff, brMeshFile->fileSize, LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);
    }

    static int compareVertices(void *userdata)
    {
        lite3d_mesh ubrMesh;
        lite3d_mesh brMesh;
        lite3d_pack *fileSysPack;
        loadMeshes(&ubrMesh, &brMesh, &fileSysPack);

        void *ubrVertexBuffer = lite3d_vbo_map(&ubrMesh.vertexBuffer, LITE3D_VBO_MAP_READ_ONLY);
        EXPECT_TRUE(ubrVertexBuffer != NULL);
        void *brVertexBuffer = lite3d_vbo_map(&brMesh.vertexBuffer, LITE3D_VBO_MAP_READ_ONLY);
        EXPECT_TRUE(brVertexBuffer != NULL);

        /* check vertex buffer size */
        EXPECT_TRUE(ubrMesh.verticesCount == brMesh.verticesCount);
        EXPECT_TRUE(ubrMesh.vertexBuffer.size == brMesh.vertexBuffer.size);
        /* check vertex buffer content */
        EXPECT_TRUE(memcmp(ubrVertexBuffer, brVertexBuffer, ubrMesh.vertexBuffer.size) == 0);
        lite3d_vbo_unmap(&ubrMesh.vertexBuffer);
        lite3d_vbo_unmap(&brMesh.vertexBuffer);

        lite3d_mesh_purge(&ubrMesh);
        lite3d_mesh_purge(&brMesh);
        lite3d_pack_close(fileSysPack);
        /* quit immediatly */
        return LITE3D_FALSE;
    }

    static int compareChunkData(void *userdata)
    {
        lite3d_mesh ubrMesh;
        lite3d_mesh brMesh;
        lite3d_pack *fileSysPack = nullptr;
        lite3d_mesh_chunk *ubrChunk = nullptr, *brChunk = nullptr;
        lite3d_list_node *ubrLink, *brLink;
        loadMeshes(&ubrMesh, &brMesh, &fileSysPack);

        /* check chunks data */
        EXPECT_TRUE(lite3d_list_count(&ubrMesh.chunks) == lite3d_list_count(&brMesh.chunks));
        for (ubrLink = ubrMesh.chunks.l.next, brLink = brMesh.chunks.l.next; ubrLink != &ubrMesh.chunks.l; 
            ubrLink = lite3d_list_next(ubrLink), brLink = lite3d_list_next(brLink))
        {
            ubrChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, ubrLink, link);
            brChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, brLink, link);

            EXPECT_TRUE(ubrChunk->vao.elementsCount == brChunk->vao.elementsCount);
            EXPECT_TRUE(ubrChunk->vao.indexesCount == brChunk->vao.indexesCount);
            EXPECT_TRUE(ubrChunk->vao.verticesCount == brChunk->vao.verticesCount);
            EXPECT_TRUE(ubrChunk->vao.verticesSize == brChunk->vao.verticesSize);
            EXPECT_TRUE(ubrChunk->vao.verticesOffset == brChunk->vao.verticesOffset);
        }

        lite3d_mesh_purge(&ubrMesh);
        lite3d_mesh_purge(&brMesh);
        lite3d_pack_close(fileSysPack);
        /* quit immediatly */
        return LITE3D_FALSE;
    }
};

LITE3D_GTEST_DECLARE(MeshCompare_Test, CompareVertices, compareVertices)
LITE3D_GTEST_DECLARE(MeshCompare_Test, CompareChunksData, compareChunkData)

#endif
