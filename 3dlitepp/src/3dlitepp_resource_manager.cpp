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
#include <iomanip>
#include <string>

#include <SDL_log.h>
#include <SDL_timer.h>

#include <3dlite/3dlite_misc.h>

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_resource_manager.h>

namespace lite3dpp
{
    ResourceManager::ResourceManager(Main *main) : 
        mMain(main)
    {

    }

    ResourceManager::~ResourceManager()
    {
        releaseAllResources();
    }

    AbstractResource *ResourceManager::fetchResource(const lite3dpp_string &key)
    {
        /* lookup resource */
        Resources::iterator it;
        if((it = mResources.find(key)) != mResources.end())
        {
            mapResource(it->second);
            return it->second;
        }

        return NULL;
    }

    void ResourceManager::loadResource(const lite3dpp_string &name,
        const lite3dpp_string &path,
        AbstractResource *resource)
    {
        size_t fileSize;
        const char *buffer;

        /* open file buffer */
        buffer = static_cast<const char *>(loadFileToMemory(path, &fileSize));
        /* load resource from memory file */
        resource->load(buffer, fileSize);

        /* map resource to GPU memory */
        mapResource(resource);
        /* just insert resource */
        mResources.insert(std::make_pair(name, resource));
    }

    void ResourceManager::mapResource(AbstractResource *resource)
    {
        resource->map();
    }

    void ResourceManager::releaseAllResources()
    {
        Resources::iterator it = mResources.begin();
        for(; it != mResources.end(); ++it)
        {
            it->second->unmap();
            it->second->unload();
            delete it->second;
        }

        mResources.clear();
    }

    void ResourceManager::releaseResource(const lite3dpp_string &name)
    {
        Resources::iterator it;
        if((it = mResources.find(name)) != mResources.end())
        {
            it->second->unmap();
            it->second->unload();
            delete it->second;

            mResources.erase(it);
        }
    }

    lite3dpp_string ResourceManager::generateName()
    {
        lite3dpp_stringstream result;
        result << std::hex << SDL_GetPerformanceCounter();
        return result.str();
    }

    ResourceManager::ResourceManagerStats ResourceManager::getStats() const
    {
        ResourceManagerStats stats;
        memset(&stats, 0, sizeof(stats));

        Packs::const_iterator packIt = mPacks.begin();
        for (; packIt != mPacks.end(); ++packIt)
        {
            stats.totalFileCacheSize += packIt->second->memoryUsed;
        }

        Resources::const_iterator resIt = mResources.begin();
        for(; resIt != mResources.end(); ++resIt)
        {
            stats.resourcesHeapSize += resIt->second->heapSize();
            stats.resourcesMappedSize += resIt->second->mappedSize();
        }

        return stats;
    }

    void ResourceManager::addResourceLocation(const lite3dpp_string &name, 
        const lite3dpp_string &path, size_t fileCacheMaxSize)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Open resource location: [%s] %s", name.c_str(), path.c_str());

        Packs::iterator it = mPacks.find(name);
        if (it != mPacks.end())
            throw std::runtime_error(lite3dpp_string("Resource location already opened: ") + 
                name + " :: " + path);

        uint8_t isFile = path.back() == '/' || path.back() == '.' ?
            LITE3D_FALSE : LITE3D_TRUE;

        lite3d_resource_pack *pack =
            lite3d_resource_pack_open(path.c_str(), isFile, fileCacheMaxSize);
        if(!pack)
            throw std::runtime_error(lite3dpp_string("Location open failed.. ") + 
                name + " :: " + path);

        mPacks.insert(std::make_pair(name, pack));
    }

    const void *ResourceManager::loadFileToMemory(const lite3dpp_string &path, size_t *size)
    {
        lite3dpp_stringstream pathStream(path);
        lite3dpp_string packName, filePath;
        std::getline(pathStream, packName, ':');
        std::getline(pathStream, filePath);
        /* lookup resource package */
        Packs::iterator packIt;
        if((packIt = mPacks.find(packName)) == mPacks.end())
            throw std::runtime_error(lite3dpp_string("Package not found: ") + packName);

        /* load resource file to memory */
        lite3d_resource_file *resourceFile =
            lite3d_resource_pack_file_load(packIt->second, filePath.c_str());
        if(!resourceFile || !resourceFile->isLoaded)
            throw std::runtime_error(lite3dpp_string("Resource open failed..") + filePath);

        *size = resourceFile->fileSize;
        return resourceFile->fileBuff;
    }
}

