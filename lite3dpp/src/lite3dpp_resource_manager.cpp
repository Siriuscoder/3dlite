/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2025 Sirius (Korolev Nikita)
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
    {}

    ResourceManager::~ResourceManager()
    {
        releaseAllResources();
        dropFileCache();
    }

    AbstractResource *ResourceManager::fetchResource(const String &key)
    {
        /* lookup resource */
        Resources::iterator it;
        if((it = mResources.find(key)) != mResources.end())
        {
            it->second->reload();
            return it->second.get();
        }

        return nullptr;
    }

    bool ResourceManager::resourceExists(const String &name)
    {
        return mResources.count(name) > 0;
    }

    void ResourceManager::loadResource(const String &name,
        const String &path,
        std::shared_ptr<AbstractResource> resource)
    {
        size_t fileSize;

        /* open file buffer */
        const char *buffer = static_cast<const char *>(loadFileToMemory(path, &fileSize));

        /* load resource from memory file */
        loadResource(name, buffer, fileSize, resource);
    }
    
    void ResourceManager::loadResource(const String &name, 
        const void *buffer, size_t size,
        std::shared_ptr<AbstractResource> resource)
    {
        /* load resource from memory chunk */
        resource->load(buffer, size);
        /* just insert resource */
        if (!mResources.emplace(name, resource).second)
        {
            LITE3D_THROW("Resource '" << name << "' already exists");
        }
    }

    void ResourceManager::releaseAllResources()
    {
        Resources::iterator it = mResources.begin();
        for(; it != mResources.end(); ++it)
        {
            it->second->unload();
        }

        mResources.clear();
    }

    void ResourceManager::releaseResource(const String &name)
    {
        Resources::iterator it;
        if((it = mResources.find(name)) != mResources.end())
        {
            it->second->unload();
            mResources.erase(it);
        }
    }
    
    void ResourceManager::dropFileCache()
    {
        for(Packs::value_type &pack : mPacks)
        {
            lite3d_pack_close(pack.second);
        }
        
        mPacks.clear();
    }
        
    void ResourceManager::dropFileCache(const String &location)
    {
        Packs::iterator it;
        if((it = mPacks.find(location)) != mPacks.end())
        {
            lite3d_pack_close(it->second);
            mPacks.erase(it);
        }        
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
            stats.totalCachedFilesMemSize += packIt->second->memoryUsed;
            stats.fileCachesCount++;
        }

        Resources::const_iterator resIt = mResources.begin();
        for (; resIt != mResources.end(); ++resIt)
        {
            stats.usedVideoMem += resIt->second->usedVideoMemBytes();
            stats.totalObjectsCount++;
            switch(resIt->second->getType())
            {
            case AbstractResource::MATERIAL:
                stats.materialsCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.materialsLoadedCount++;
                break;
            case AbstractResource::TEXTURE:
                stats.texturesCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.texturesLoadedCount++;
                break;
            case AbstractResource::SCENE:
                stats.scenesCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.scenesLoadedCount++;
                break;
            case AbstractResource::MESH:
                stats.meshesCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.meshesLoadedCount++;
                break;                
            case AbstractResource::RENDER_TARGET:
                stats.renderTargetsCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.renderTargetsLoadedCount++;
                break;
            case AbstractResource::SHADER_PROGRAM:
                stats.shaderProgramsCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.shaderProgramsLoadedCount++;
                break;
            case AbstractResource::SHADER_STORAGE:
                stats.ssboCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.ssboLoadedCount++;
                break;
            case AbstractResource::UNIFORM_BUFFER:
                stats.uboCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.uboLoadedCount++;
                break;
            case AbstractResource::PIPELINE:
                stats.pipelinesCount++;
                break;
            case AbstractResource::MESH_PARTITION:
                stats.meshPartitionsCount++;
                if (resIt->second->getState() == AbstractResource::LOADED)
                    stats.meshPartitionsLoadedCount++;
                break;
            }
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
            LITE3D_THROW("Resource location already opened: " << 
                name << " :: " << path);

        uint8_t isFile = path.back() == '/' || path.back() == '.' ?
            LITE3D_FALSE : LITE3D_TRUE;

        lite3d_pack *pack =
            lite3d_pack_open(path.c_str(), isFile, fileCacheMaxSize);
        if(!pack)
            LITE3D_THROW("Location open failed.. " << 
                name << " :: " << path);

        mPacks.emplace(name, pack);
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
        String packageName, filePath;
        std::getline(pathStream, packageName, ':');
        std::getline(pathStream, filePath);
        /* lookup resource package */
        Packs::iterator packIt;

        if(filePath.empty())
            filePath.swap(packageName);
        else if((packIt = mPacks.find(packageName)) != mPacks.end())
            mLastUsed = packIt->second;
        else
            LITE3D_THROW("Package not found: \"" << packageName << "\" while loading path \"" << path << "\"");

        if(!mLastUsed)
            LITE3D_THROW("Package not specified: \"" << path << "\"");

        /* load resource file to memory */
        lite3d_file *resourceFile =
            lite3d_pack_file_load(mLastUsed, filePath.c_str());
        if(!resourceFile || !resourceFile->isLoaded)
            LITE3D_THROW("File open error..." << "\"" << path << "\"");

        return resourceFile;
    }

    void ResourceManager::warmUpMeshPartitions()
    {
        Resources::const_iterator it = mResources.begin();
        for (; it != mResources.end(); ++it)
        {
            if (it->second->getType() == AbstractResource::MESH_PARTITION)
            {
                MeshPartition *meshPartition = static_cast<MeshPartition *>(it->second.get());
                meshPartition->warmUpMemory();
            }
        }
    }
}

