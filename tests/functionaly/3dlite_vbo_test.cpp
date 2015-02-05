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

#include <3dlite/GL/glew.h>
#include "3dlite_common_test.h"

class VBO_Test : public ::testing::Test
{
public:

protected:
    Lite3dCommon mlite3dCommon;
};

static int meshLoadingTest()
{
    lite3d_vbo mVBO;
    lite3d_resource_pack *fileSysPack = lite3d_resource_pack_open("tests/", 0, 1000000);
    EXPECT_TRUE(fileSysPack != NULL);
    lite3d_resource_file *meshFile = lite3d_resource_pack_file_load(fileSysPack, 
        "pack/minigun.3ds");
    EXPECT_TRUE(lite3d_vbo_init(&mVBO) == LITE3D_TRUE);
    EXPECT_TRUE(lite3d_vbo_load(&mVBO, meshFile, NULL, GL_STATIC_DRAW) == LITE3D_TRUE);


    lite3d_vbo_purge(&mVBO);
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
