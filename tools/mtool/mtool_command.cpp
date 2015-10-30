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
#pragma once

#include <mtool_command.h>

Command::Command()
{
    mMain.registerLifecycleListener(this);
}

void Command::setInputFilePath(const lite3dpp::String &filePath)
{
    mInputFilePath.assign("filesystem:");
    mInputFilePath.append(filePath);
}
    
void Command::run()
{
    lite3dpp::ConfigurationWriter writer;

    writer.set(L"LogLevel", 1);
    writer.set(L"LogFlushAlways", false);

    lite3dpp::ConfigurationWriter video;
    video.set(L"Width", 800);
    video.set(L"Height", 600);
    video.set(L"Caption", "Dummy");
    video.set(L"ColorBits", 24);
    video.set(L"Fullscreen", false);
    video.set(L"Hidden", true);

    lite3dpp::ConfigurationWriter texture;
    texture.set(L"Compression", false);

    lite3dpp::ConfigurationWriter reslocation;
    reslocation.set(L"Name", "filesystem");
    reslocation.set(L"Path", "./");
    reslocation.set(L"FileCacheMaxSize", 0x6400000);

    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector reslocationArr;
    reslocationArr.push_back(reslocation);
    writer.set(L"ResourceLocations", reslocationArr);
    writer.set(L"VideoSettings", video);
    writer.set(L"TextureSettings", texture);

    lite3dpp::String code = writer.write();

    mMain.initFromConfigString(writer.write().c_str());
    writer.clear();
    mMain.run();
}

void Command::init()
{
    runImpl();
    mMain.stop();
}

void Command::shut()
{}

void Command::frameBegin()
{}

void Command::frameEnd()
{}

void Command::timerTick(lite3d_timer *timerid)
{}

void Command::processEvent(SDL_Event *e)
{}



