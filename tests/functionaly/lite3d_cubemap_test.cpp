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
#include <lite3d/lite3d_texture_unit.h>
#include "lite3d_common_test.h"

class CubemapLoadingTest : public Lite3dCommon
{
public:

    static int cubemapLoadingTest(void *userdata)
    {
        lite3d_texture_unit cubemap = {};
        lite3d_pack *fileSysPack = lite3d_pack_open("samples/", 0, 1000000);
        EXPECT_TRUE(fileSysPack);
        if (!fileSysPack)
            return LITE3D_FALSE;

        lite3d_file *ddsFile = lite3d_pack_file_load(fileSysPack, "textures/images/skymap/Vault111CryoCube.dds");
        EXPECT_TRUE(ddsFile);
        if (!ddsFile)
            return LITE3D_FALSE;

        EXPECT_TRUE(lite3d_texture_unit_from_resource(&cubemap, ddsFile,
            LITE3D_IMAGE_DDS,
            LITE3D_TEXTURE_CUBE, 
            LITE3D_FALSE,
            LITE3D_TEXTURE_FILTER_TRILINEAR, 
            LITE3D_TEXTURE_CLAMP_TO_EDGE, 
            0));

        lite3d_texture_unit_purge(&cubemap);
        lite3d_pack_close(fileSysPack);
        /* quit immediatly */
        return LITE3D_FALSE;
    }
};

LITE3D_GTEST_DECLARE(CubemapLoadingTest, cubemapLoading, cubemapLoadingTest)
