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
#include <gtest/gtest.h>

#include <3dlite/3dlite_m_codec.h>
#include "3dlite_common_test.h"

class VBO_Test : public ::testing::Test
{
public:

protected:
    Lite3dCommon mlite3dCommon;
};

static int meshLoadingTest(void *userdata)
{
    lite3d_indexed_mesh mVBO;
    lite3d_resource_pack *fileSysPack = lite3d_resource_pack_open("tests/", 0, 1000000);
    EXPECT_TRUE(fileSysPack != NULL);
    lite3d_resource_file *meshFile = lite3d_resource_pack_file_load(fileSysPack, 
        "pack/minigun/minigun.3ds");
    EXPECT_TRUE(lite3d_indexed_mesh_init(&mVBO) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_indexed_mesh_load(&mVBO, meshFile, NULL, LITE3D_VBO_STATIC_DRAW, 0) == LITE3D_TRUE);


    lite3d_indexed_mesh_purge(&mVBO);
    lite3d_resource_pack_close(fileSysPack);
    /* quit immediatly */
    return LITE3D_FALSE;
}

static int encodeDecode_M_formatTest(void *userdata)
{
    lite3d_indexed_mesh mVBO;
    lite3d_resource_pack *fileSysPack = lite3d_resource_pack_open("tests/", 0, 1000000);
    EXPECT_TRUE(fileSysPack != NULL);
    lite3d_resource_file *meshFile = lite3d_resource_pack_file_load(fileSysPack, 
        "pack/minigun/minigun.3ds");
    EXPECT_TRUE(lite3d_indexed_mesh_init(&mVBO) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_indexed_mesh_load(&mVBO, meshFile, NULL, LITE3D_VBO_STATIC_DRAW, 
        LITE3D_OPTIMIZE_MESH_FLAG) == LITE3D_TRUE);

    size_t mfileSize = lite3d_indexed_mesh_m_encode_size(&mVBO);
    void *encodeBuffer = lite3d_malloc(mfileSize);
    EXPECT_TRUE(lite3d_indexed_mesh_m_encode(&mVBO, encodeBuffer, mfileSize) == LITE3D_TRUE);

    lite3d_free(encodeBuffer);
    lite3d_indexed_mesh_purge(&mVBO);
    lite3d_resource_pack_close(fileSysPack);
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
