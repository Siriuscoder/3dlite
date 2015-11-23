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
#define MAKE_PATH(folder, name) (folder + "/" + name).c_str()

#ifdef PLATFORM_Windows
#   include <Windows.h>
#   define MKDIR(folder, name) \
    if(CreateDirectory(MAKE_PATH(folder, name), NULL)) \
        printf("Make directory %s\n", MAKE_PATH(folder, name));
#elif PLATFORM_Linux
#   include <sys/stat.h>
#   define MKDIR(folder, name) \
    if(mkdir(MAKE_PATH(folder, name), 755) == 0) \
        printf("Make directory %s\n", MAKE_PATH(folder, name));
#endif

#include <SDL_rwops.h>
#include <mtool_command.h>

Command::Command() : 
    mNonameCounter(0)
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

void Command::saveFile(const void *buffer, size_t size, const lite3dpp::String &path)
{
    SDL_RWops *descr;
    printf("Writing %s ... ", path.c_str());
    fflush(stdout);

    descr = SDL_RWFromFile(path.c_str(), "wb");
    if (!descr)
        throw std::runtime_error(lite3dpp::String("Unable to open file " + path));

    if (SDL_RWwrite(descr, buffer, size, 1) != 1)
    {
        SDL_RWclose(descr);
        throw std::runtime_error(lite3dpp::String("IO error.. ") + path);
    }

    SDL_RWclose(descr);

    printf("done\n");
    fflush(stdout);
}

lite3dpp::String Command::makeFullPath(const lite3dpp::String &outputFolder, const lite3dpp::String &relative)
{
    char path[1024] = {0};
    sprintf(path, "%s%s%s", outputFolder.c_str(), outputFolder.size() == 0 ? "" : "/", relative.c_str());

    return path;
}

lite3dpp::String Command::makeRelativePath(const lite3dpp::String &inpath, const lite3dpp::String &name,
    const lite3dpp::String &ext)
{
    char path[1024] = {0};

    if (name.size() == 0)
        sprintf(path, "%snoname%d.%s", inpath.c_str(), ++mNonameCounter, ext.c_str());
    else
        sprintf(path, "%s%s.%s", inpath.c_str(), name.c_str(), ext.c_str());

    return path;
}

void Command::makeFolders(const lite3dpp::String &outputFolder)
{
    MKDIR(outputFolder, "materials");
    MKDIR(outputFolder, "models");
    MKDIR(outputFolder, "models/json");
    MKDIR(outputFolder, "models/meshes");
    MKDIR(outputFolder, "objects");
    MKDIR(outputFolder, "scenes");
    MKDIR(outputFolder, "scripts");
    MKDIR(outputFolder, "shaders");
    MKDIR(outputFolder, "shaders/json");
    MKDIR(outputFolder, "shaders/sources");
    MKDIR(outputFolder, "targets");
    MKDIR(outputFolder, "textures");
    MKDIR(outputFolder, "textures/json");
    MKDIR(outputFolder, "textures/images");
}



