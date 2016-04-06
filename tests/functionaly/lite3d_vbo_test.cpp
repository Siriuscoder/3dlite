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
#include <gtest/gtest.h>

#include <lite3d/lite3d_mesh_codec.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>
#include "lite3d_common_test.h"

#ifdef INCLUDE_ASSIMP

class VBO_Test : public ::testing::Test
{
public:

protected:
    Lite3dCommon mlite3dCommon;
};

static int meshLoadingTest(void *userdata)
{
    lite3d_mesh mVBO;
    lite3d_pack *fileSysPack = lite3d_pack_open("tests/", 0, 1000000);
    EXPECT_TRUE(fileSysPack != NULL);
    lite3d_file *meshFile = lite3d_pack_file_load(fileSysPack,
                                                                    "pack/minigun/minigun.3ds");
    EXPECT_TRUE(lite3d_mesh_init(&mVBO) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_assimp_mesh_load(&mVBO, meshFile, NULL, LITE3D_VBO_STATIC_DRAW, 0) == LITE3D_TRUE);


    lite3d_mesh_purge(&mVBO);
    lite3d_pack_close(fileSysPack);
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
    EXPECT_TRUE(lite3d_mesh_init(&mVBO) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_assimp_mesh_load(&mVBO, meshFile, NULL, LITE3D_VBO_STATIC_DRAW,
        LITE3D_OPTIMIZE_MESH_FLAG) == LITE3D_TRUE);

    size_t mfileSize = lite3d_mesh_m_encode_size(&mVBO);
    void *encodeBuffer = lite3d_malloc(mfileSize);
    EXPECT_TRUE(lite3d_mesh_m_encode(&mVBO, encodeBuffer, mfileSize) == LITE3D_TRUE);

    EXPECT_TRUE(lite3d_mesh_init(&mVBO1) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_mesh_m_decode(&mVBO1, encodeBuffer, mfileSize, 
        LITE3D_VBO_STATIC_DRAW) == LITE3D_TRUE);


    lite3d_free(encodeBuffer);
    lite3d_mesh_purge(&mVBO);
    lite3d_mesh_purge(&mVBO1);
    lite3d_pack_close(fileSysPack);
    /* quit immediatly */
    return LITE3D_FALSE;
}

TEST_F(VBO_Test, meshLoading)
{
    mlite3dCommon.settings().renderLisneters.preRender = meshLoadingTest;
    mlite3dCommon.main();
}

TEST_F(VBO_Test, mapping)
{

}

TEST_F(VBO_Test, encodeDecode_M_format)
{
    mlite3dCommon.settings().renderLisneters.preRender = encodeDecode_M_formatTest;
    mlite3dCommon.main();
}

#endif
