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
#include <iomanip>
#include <string>

#include <SDL_log.h>
#include <SDL_timer.h>

#include <lite3d/lite3d_misc.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_resource_manager.h>

namespace lite3dpp
{
    ResourceManager::ResourceManager(Main *main) : 
        mMain(main)
    {

    }

    ResourceManager::~ResourceManager()
    {
        releaseAllResources();
        releaseFileCache();
    }

    AbstractResource *ResourceManager::fetchResource(const String &key)
    {
        /* lookup resource */
        Resources::iterator it;
        if((it = mResources.find(key)) != mResources.end())
        {
            it->second->reload();
            return it->second;
        }

        return NULL;
    }

    void ResourceManager::loadResource(const String &name,
        const String &path,
        AbstractResource *resource)
    {
        size_t fileSize;

        /* open file buffer */
        const char *buffer = static_cast<const char *>(loadFileToMemory(path, &fileSize));

        /* load resource from memory file */
        loadResource(name, buffer, fileSize, resource);
    }
    
    void ResourceManager::loadResource(const String &name, 
        const void *buffer, size_t size,
        AbstractResource *resource)
    {
        /* load resource from memory chunk */
        resource->load(buffer, size);

        /* just insert resource */
        mResources.insert(std::make_pair(name, resource));     
    }

    void ResourceManager::releaseAllResources()
    {
        Resources::iterator it = mResources.begin();
        for(; it != mResources.end(); ++it)
        {
            it->second->unload();
            delete it->second;
        }

        mResources.clear();
    }

    void ResourceManager::releaseResource(const String &name)
    {
        Resources::iterator it;
        if((it = mResources.find(name)) != mResources.end())
        {
            it->second->unload();
            delete it->second;

            mResources.erase(it);
        }
    }
    
        void ResourceManager::releaseFileCache()
    {
        for(Packs::value_type &pack : mPacks)
        {
            lite3d_pack_close(pack.second);
        }
        
        mPacks.clear();
    }

    String ResourceManager::generateResourceName()
    {
        Stringstream result;
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
            stats.bufferedSize += resIt->second->getBufferedSize();
        }

        return stats;
    }

    void ResourceManager::addResourceLocation(const String &name, 
        const String &path, size_t fileCacheMaxSize)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Open resource location: [%s] %s", name.c_str(), path.c_str());

        Packs::iterator it = mPacks.find(name);
        if (it != mPacks.end())
            throw std::runtime_error(String("Resource location already opened: ") + 
                name + " :: " + path);

        uint8_t isFile = path.back() == '/' || path.back() == '.' ?
            LITE3D_FALSE : LITE3D_TRUE;

        lite3d_pack *pack =
            lite3d_pack_open(path.c_str(), isFile, fileCacheMaxSize);
        if(!pack)
            throw std::runtime_error(String("Location open failed.. ") + 
                name + " :: " + path);

        mPacks.insert(std::make_pair(name, pack));
    }

    const void *ResourceManager::loadFileToMemory(const String &path, size_t *size)
    {
        /* load resource file to memory */
        const lite3d_file *resourceFile = loadFileToMemory(path);
        *size = resourceFile->fileSize;
        return resourceFile->fileBuff;
    }

    const lite3d_file *ResourceManager::loadFileToMemory(const String &path)
    {
        Stringstream pathStream(path);
        String packName, filePath;
        std::getline(pathStream, packName, ':');
        std::getline(pathStream, filePath);
        /* lookup resource package */
        Packs::iterator packIt;
        if((packIt = mPacks.find(packName)) == mPacks.end())
            throw std::runtime_error(String("Package not found: ") + packName + ", \"" + path + "\"");

        /* load resource file to memory */
        lite3d_file *resourceFile =
            lite3d_pack_file_load(packIt->second, filePath.c_str());
        if(!resourceFile || !resourceFile->isLoaded)
            throw std::runtime_error(String("File not found.. ") + "\"" + path + "\"");

        return resourceFile;
    }
}

