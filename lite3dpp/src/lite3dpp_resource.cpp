/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2026 Sirius (Korolev Nikita)
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
#include <algorithm>
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_resource_manager.h>

const lite3dpp::String lite3dpp::ConfigurableResource::emptyJson = LITE3D_EMPTY_JSON;

namespace lite3dpp
{
    const char *AbstractResource::ResourceTypeName[] = {
        "SCRIPT",
        "MESH",
        "SCENE",
        "MATERIAL",
        "TEXTURE",
        "SHADER_PROGRAM",
        "RENDER_TARGET",
        "SHADER_STORAGE",
        "UNIFORM_BUFFER",
        "PIPELINE",
        "MESH_PARTITION",
        "ACTION"
    };
    
    AbstractResource::AbstractResource(const String &name, const String &path, Main &main, 
        ResourceType type) : 
        mState(ResourceState::UNLOADED),
        mType(type),
        mName(name),
        mPath(path),
        mMain(main),
        mPinned(false)
    {}

    AbstractResource::~AbstractResource()
    {
        for (auto resource : mParentResources)
        {
            resource->removeChildResource(this);
        }

        for (auto resource : mChildResources)
        {
            resource->removeParentResource(this);
        }
    }

    void AbstractResource::addParentResource(AbstractResource *parent)
    {
        if (!parent)
            return;

        if (std::find(mParentResources.begin(), mParentResources.end(), parent) == mParentResources.end())
        {
            mParentResources.push_back(parent);
            parent->addChildResource(this);
        }
    }

    void AbstractResource::removeParentResource(AbstractResource *parent)
    {
        SDL_assert(parent);

        auto it = std::find(mParentResources.begin(), mParentResources.end(), parent);
        if (it != mParentResources.end())
        {
            mParentResources.erase(it);
        }
    }

    void AbstractResource::unloadBranch()
    {
        for (auto it = mChildResources.rbegin(); it != mChildResources.rend(); ++it)
        {
            (*it)->unloadBranch();
            (*it)->unload();
        }
    }
    
    void AbstractResource::logState()
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Resource %s %s \"%s\" %s", ResourceTypeName[mType], 
                mName.c_str(), 
                mPath.size() == 0 ? "" : mPath.c_str(), 
                mState == LOADED ? "LOADED" : "UNLOADED");
    }

    void AbstractResource::load(const void *buffer, size_t size)
    {
        if (mState == UNLOADED)
        {
            try
            {
                loadImpl(buffer, size);
                mState = LOADED;
            }
            catch (const std::exception&)
            {
                unloadImpl();
                throw;
            }
            
            logState();
        }
    }

    void AbstractResource::reload()
    {
        if (mState == UNLOADED)
        {
            try
            {
                loadImpl(nullptr, 0);
                mState = LOADED;
            }
            catch (const std::exception&)
            {
                unloadImpl();
                throw;
            }
            
            logState();
        }
    }

    void AbstractResource::unload()
    {
        if(mState == LOADED)
        {
            unloadImpl();
            mState = UNLOADED;
            
            logState();
        }
    }

    size_t AbstractResource::usedVideoMemBytes() const
    {
        return 0;
    }

    size_t AbstractResource::fullVideoMemBytes() const 
    {
        return 0;
    }

    void AbstractResource::addChildResource(AbstractResource *resource)
    {
        SDL_assert(resource);
        SDL_assert(std::find(mChildResources.begin(), mChildResources.end(), resource) == mChildResources.end());
        mChildResources.push_back(resource);
    }

    void AbstractResource::removeChildResource(AbstractResource *resource)
    {
        SDL_assert(resource);

        auto it = std::find(mChildResources.begin(), mChildResources.end(), resource);
        if (it != mChildResources.end())
        {
            mChildResources.erase(it);
        }
    }

    ConfigurableResource::ConfigurableResource(const String &name, const String &path, 
        Main &main, ResourceType type) : 
        AbstractResource(name, path, main, type)
    {}

    ConfigurableResource::~ConfigurableResource()
    {}

    void ConfigurableResource::loadImpl(const void *buffer, size_t size)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "%s resource (%s) \"%s\" ...", buffer ? "Loading" : "Reloading", getName().c_str(), 
            getPath().size() == 0 ? "" : getPath().c_str()); 

        if (buffer)
        {
            mConfiguration.reset(new ConfigurationReader(static_cast<const char *>(buffer), size));
        }

        loadFromConfigImpl(*mConfiguration);
    }

    void ConfigurableResource::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        /* by default, we try to load resource from json one more time */
        loadFromConfigImpl(helper);
    }

    const ConfigurationReader &ConfigurableResource::getConfig() const
    {
        SDL_assert(mConfiguration);
        return *mConfiguration;
    }
}

