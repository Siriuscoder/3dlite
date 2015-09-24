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
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_resource_manager.h>

namespace lite3dpp
{
    const char *AbstractResource::ResourceTypeName[] = {
        "SCRIPT",
        "MESH",
        "SCENE",
        "MATERIAL",
        "TEXTURE",
        "SHADER_PROGRAM",
        "RENDER_TARGET"
    };
    
    AbstractResource::AbstractResource(const String &name,
        const String &path, Main *main, ResourceType type) : 
        mState(UNLOADED),
        mType(type),
        mName(name),
        mPath(path),
        mBufferedSize(0),
        mMain(main)
    {}

    AbstractResource::~AbstractResource()
    {}
    
    void AbstractResource::logState()
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Resource %s %s \"%s\" %s", ResourceTypeName[mType], 
                mName.c_str(), 
                mPath.size() == 0 ? "from memory" : mPath.c_str(), 
                mState == LOADED ? "LOADED" : "UNLOADED");
    }

    void AbstractResource::load(const void *buffer, size_t size)
    {
        if(mState == UNLOADED)
        {
            loadImpl(buffer, size);
            mState = LOADED;
            
            logState();
        }
    }

    void AbstractResource::reload()
    {
        if(mState != LOADED)
        {
            reloadImpl();
            mState = LOADED;
            
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

    JsonResource::JsonResource(const String &name, 
        const String &path, Main *main, ResourceType type) : 
        AbstractResource(name, path, main, type)
    {}

    JsonResource::~JsonResource()
    {}

    void JsonResource::loadImpl(const void *buffer, size_t size)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Parsing json (%s) \"%s\" ...", getName().c_str(), 
            getPath().size() == 0 ? "from memory" : getPath().c_str()); 

        mJsonHelper.reset(new JsonHelper(static_cast<const char *>(buffer), size));
        reloadImpl();
    }

    void JsonResource::reloadImpl()
    {
        SDL_assert_release(mJsonHelper);
        loadFromJsonImpl(*mJsonHelper);
    }

    const JsonHelper &JsonResource::getJson() const
    {
        SDL_assert_release(mJsonHelper);
        return *mJsonHelper;
    }

    NoncopiableResource::NoncopiableResource()
    {}

    NoncopiableResource::NoncopiableResource(const NoncopiableResource &other)
    {}
}

