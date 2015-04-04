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
#include <SDL_log.h>

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_resource_pack_manager.h>

namespace lite3dpp
{
    ResourcePackManager::ResourcePackManager(Main *mainObj) :
        mMainObj(mainObj)
    {

    }

    ResourcePackManager::~ResourcePackManager()
    {

    }

    lite3d_resource_file *ResourcePackManager::loadResourceFromFile(const lite3dpp_string &fileName)
    {
        PacksMap::iterator it = mPacks.begin();
        for (; it != mPacks.end(); ++it)
        {
            lite3d_resource_file *resource =
                lite3d_resource_pack_file_load(it->second, fileName.c_str());

            if (resource)
                return resource;
        }

        throw std::runtime_error((fileName + ": open failed..").c_str());
    }

    void ResourcePackManager::unloadResource(lite3d_resource_file *resource)
    {
        lite3d_resource_pack_file_purge(resource);
    }

    void ResourcePackManager::unloadResource(const lite3dpp_string &resourceName)
    {
        PacksMap::iterator it = mPacks.begin();
        for (; it != mPacks.end(); ++it)
        {
            lite3d_resource_file *resource =
                lite3d_resource_pack_file_find(it->second, resourceName.c_str());

            if(resource)
                unloadResource(resource);
        }
    }

    void ResourcePackManager::unloadAllResources()
    {
        PacksMap::iterator it = mPacks.begin();
        for (; it != mPacks.end(); ++it)
        {
            lite3d_resource_pack_close(it->second);
        }

        mPacks.clear();
    }

    size_t ResourcePackManager::loadedResourcesSize() const
    {
        size_t result = 0;
        PacksMap::const_iterator it = mPacks.begin();
        for (; it != mPacks.end(); ++it)
        {
            result += it->second->memoryUsed;
        }

        return result;
    }

    lite3d_resource_pack *ResourcePackManager::addResourceLocation(const lite3dpp_string &path)
    {
        PacksMap::iterator it = mPacks.find(path);
        if (it != mPacks.end())
            return it->second;

        uint8_t isFile = path.back() == '/' || path.back() == '.' ?
LITE3D_FALSE : LITE3D_TRUE;

        lite3d_resource_pack *pack =
            lite3d_resource_pack_open(path.c_str(), isFile, 0);
        if (mPacks.insert(std::make_pair(path, pack)).second)
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Resource location added: %s", path.c_str());

        return pack;
    }

    void ResourcePackManager::init()
    {

    }

    void ResourcePackManager::shut()
    {
        unloadAllResources();
    }
}