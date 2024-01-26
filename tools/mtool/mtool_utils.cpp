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
#define MAKE_PATH(folder, name) (folder + "/" + name).c_str()
#include <algorithm>
#include <cctype>

#ifdef _MSC_VER
#   include <Windows.h>
#   define MKDIR(folder, name) \
    if(CreateDirectory(MAKE_PATH(folder, name), NULL)) \
        printf("Make directory %s\n", MAKE_PATH(folder, name));
#elif defined(__GNUC__) || defined(__clang__)
#   include <sys/stat.h>
#   define MKDIR(folder, name) \
    if(mkdir(MAKE_PATH(folder, name), 0755) == 0) \
        printf("Make directory %s\n", MAKE_PATH(folder, name));
#endif

#include <SDL_rwops.h>
#include <SDL_log.h>
#include <mtool/mtool_utils.h>

int Utils::mNonameCounter = 0;

void Utils::saveFile(const void *buffer, size_t size, const lite3dpp::String &path)
{
    SDL_RWops *descr;
    printf("Writing %s ... ", path.c_str());
    fflush(stdout);

    descr = SDL_RWFromFile(path.c_str(), "wb");
    if (!descr)
        LITE3D_THROW("Unable to open file " << path);

    if (SDL_RWwrite(descr, buffer, size, 1) != 1)
    {
        SDL_RWclose(descr);
        LITE3D_THROW("IO error.. " << path);
    }

    SDL_RWclose(descr);

    printf("done\n");
    fflush(stdout);
}

void Utils::saveTextFile(const lite3dpp::String &text, const lite3dpp::String &path)
{
    saveFile(text.data(), text.size(), path);
}

lite3dpp::String Utils::makeFullPath(const lite3dpp::String &outputFolder, const lite3dpp::String &relative)
{
    char path[1024] = {0};
    sprintf(path, "%s%s%s", outputFolder.c_str(), outputFolder.size() == 0 ? "" : "/", relative.c_str());

    return path;
}

lite3dpp::String Utils::makeRelativePath(const lite3dpp::String &inpath, const lite3dpp::String &name,
    const lite3dpp::String &ext)
{
    char path[1024] = {0};

    if (name.size() == 0)
        sprintf(path, "%snoname%d.%s", inpath.c_str(), ++mNonameCounter, ext.c_str());
    else
        sprintf(path, "%s%s.%s", inpath.c_str(), name.c_str(), ext.c_str());

    return path;
}

void Utils::makeFolder(const lite3dpp::String &outputFolder, const lite3dpp::String &name)
{
    MKDIR(outputFolder, name);
}

lite3dpp::String Utils::getFileExt(const lite3dpp::String &filePath)
{
    std::size_t dotPos = filePath.find(".");
    if(dotPos != lite3dpp::String::npos)
    {
        return filePath.substr(dotPos+1);
    }

    return filePath;
}

lite3dpp::String Utils::getFileNameWithoutExt(const lite3dpp::String &filePath)
{
    std::size_t dotPos = filePath.find(".");
    if(dotPos != lite3dpp::String::npos)
    {
        lite3dpp::String fileName = filePath.substr(0, dotPos);
        std::size_t slashPos = fileName.find_last_of("\\/");
        if(slashPos != lite3dpp::String::npos)
        {
            return fileName.substr(slashPos+1);
        }
    }

    return filePath;
}

lite3dpp::String Utils::extractMeshName(const lite3dpp::String &nodeName)
{
    size_t dPos = nodeName.find('_');
    if (dPos != lite3dpp::String::npos)
    {
        lite3dpp::String mn = nodeName.substr(0, dPos);
        if (mn.size() == 0)
            return nodeName;
        return mn;
    }

    return nodeName;
}
