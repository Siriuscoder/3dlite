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
#include <SDL_assert.h>

#include <3dlitepp/3dlitepp_resource.h>
#include <3dlitepp/3dlitepp_resource_manager.h>

namespace lite3dpp
{
    AbstractResource::AbstractResource(const lite3dpp_string &name,
        const lite3dpp_string &path, Main *main, ResourceType type) : 
        mState(UNLOADED),
        mType(type),
        mName(name),
        mPath(path),
        mBufferedSize(0),
        mMain(main)
    {}

    AbstractResource::~AbstractResource()
    {}

    void AbstractResource::load(const void *buffer, size_t size)
    {
        if(mState == UNLOADED)
        {
            loadImpl(buffer, size);
            mState = LOADED;
        }
    }

    void AbstractResource::reload()
    {
        if(mState != LOADED)
        {
            reloadImpl();
        }
    }

    void AbstractResource::unload()
    {
        if(mState == LOADED)
        {
            unloadImpl();
            mState = UNLOADED;
        }
    }

    JsonResource::JsonResource(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main, ResourceType type) : 
        AbstractResource(name, path, main, type)
    {}

    JsonResource::~JsonResource()
    {
        if(mJsonHelper)
        {
            delete mJsonHelper;
            mJsonHelper = NULL;
        }
    }

    void JsonResource::loadImpl(const void *buffer, size_t size)
    {
        mJsonHelper = new JsonHelper(static_cast<const char *>(buffer), size);
        reloadImpl();
    }

    void JsonResource::reloadImpl()
    {
        loadFromJsonImpl(*mJsonHelper);
    }

    const JsonHelper &JsonResource::getJson() const
    {
        SDL_assert(mJsonHelper);
        return *mJsonHelper;
    }

    NoncopiableResource::NoncopiableResource()
    {}

    NoncopiableResource::NoncopiableResource(const NoncopiableResource &other)
    {}
}

