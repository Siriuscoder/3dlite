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
#include <gtest/gtest.h>


#include <lite3dpp/lite3dpp_main.h>

class Lite3dpp_ConfWriterTest : public ::testing::Test
{
protected:

    static void SetUpTestCase()
    {
        /* setup memory */
        lite3d_memory_init(NULL);
    }

protected:

    lite3dpp::Main mMain;
};

TEST_F(Lite3dpp_ConfWriterTest, ConfigGenerate)
{
    lite3dpp::ConfigurationWriter reslocation;
    reslocation.set(L"Name", L"samples")
        .set(L"Path", L"samples/")
        .set(L"FileCacheMaxSize", 1024000);

    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector reslocationArr;
    reslocationArr.push_back(reslocation);

    lite3dpp::String code = lite3dpp::ConfigurationWriter().set(L"LogLevel", 2)
        .set(L"LogFlushAlways", false)
        .set(L"FixedUpdatesInterval", 30)
        .set(L"VideoSettings", lite3dpp::ConfigurationWriter()
            .set(L"Width", 800)
            .set(L"Height", 600)
            .set(L"Caption", L"My test window")
            .set(L"ColorBits", 24)
            .set(L"Fullscreen", false)
            .set(L"MSAA", 4)
            .set(L"VSync", true))
        .set(L"TextureSettings", lite3dpp::ConfigurationWriter()
            .set(L"Anisotropy", 8)
            .set(L"Compression", true))
        .set(L"ResourceLocations", reslocationArr).write();

    EXPECT_NO_THROW(mMain.initFromConfigString(code.c_str()));
}

