/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
    lite3dpp::ConfigurationWriter writer;

    writer.setInt(L"LogLevel", 2);
    writer.setBool(L"LogFlushAlways", false);
    writer.setInt(L"FixedUpdatesInterval", 30);

    lite3dpp::ConfigurationWriter video;
    video.setInt(L"Width", 800);
    video.setInt(L"Height", 600);
    video.setString(L"Caption", L"My test window");
    video.setInt(L"ColorBits", 24);
    video.setBool(L"Fullscreen", false);
    video.setInt(L"FSAA", 4);
    video.setBool(L"VSync", true);

    lite3dpp::ConfigurationWriter texture;
    texture.setInt(L"Anisotropy", 8);
    texture.setBool(L"Compression", true);

    lite3dpp::ConfigurationWriter reslocation;
    reslocation.setString(L"Name", L"samples");
    reslocation.setString(L"Path", L"samples/");
    reslocation.setInt(L"FileCacheMaxSize", 1024000);

    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector reslocationArr;
    reslocationArr.push_back(reslocation);
    writer.setObjects(L"ResourceLocations", reslocationArr);
    writer.setObject(L"VideoSettings", video);
    writer.setObject(L"TextureSettings", texture);

    lite3dpp::String code = writer.write();
    writer.clear();

    EXPECT_NO_THROW(mMain.initFromConfigString(code.c_str()));
}

