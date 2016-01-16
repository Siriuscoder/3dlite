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
#include <SDL_log.h>
#include <mtool_utils.h>

int Utils::mNonameCounter = 0;

void Utils::saveFile(const void *buffer, size_t size, const lite3dpp::String &path)
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

