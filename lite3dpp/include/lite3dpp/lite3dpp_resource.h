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
#pragma once

#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_json_helper.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ResourceParameters : public Manageable
    {
    public:
    };
    
    class LITE3DPP_EXPORT AbstractResource : public Manageable
    {
    public:
        
        static const char *ResourceTypeName[];

        enum ResourceState
        {
            /* just created */
            UNLOADED,
            /* fully loaded */
            LOADED
        };

        enum ResourceType : int32_t
        {
            SCRIPT = 0,
            MESH,
            SCENE,
            MATERIAL,
            TEXTURE,
            SHADER_PROGRAM,
            RENDER_TARGET
        };

        AbstractResource(const String &name, 
            const String &path, Main *main, ResourceType type);
        virtual ~AbstractResource();

        void load(const void *buffer, size_t size);
        void load(const ResourceParameters &params);
        /* call this to reload object */ 
        void reload();
        void unload();

        /* allocated size in video memory (when resource LOADED_MAPPED) */
        inline size_t getBufferedSize() const
        { return mBufferedSize; }

        inline ResourceState getState() const
        { return mState; }
        inline ResourceType getType() const
        { return mType; }
        inline const String &getName() const
        { return mName; }
        inline const String &getPath() const
        { return mPath; }
        
    protected:

        virtual void loadImpl(const void *buffer, size_t size) = 0;
        virtual void loadImpl(const ResourceParameters &params) = 0;
        virtual void reloadImpl() = 0;
        virtual void unloadImpl() = 0;

        inline void setBufferedSize(size_t size)
        { mBufferedSize = size; }

    private:
        
        void logState();

        ResourceState mState;
        ResourceType mType;
        String mName;
        String mPath;
        size_t mBufferedSize;

    protected:
        Main *mMain;
    };

    class LITE3DPP_EXPORT JsonResource : public AbstractResource
    {
    public:
        JsonResource(const String &name, 
            const String &path, Main *main, ResourceType type);
        virtual ~JsonResource();

        const JsonHelper &getJson() const;

    protected:

        virtual void loadImpl(const void *buffer, size_t size) override final;
        virtual void reloadImpl() override final;

        virtual void loadFromJsonImpl(const JsonHelper &helper) = 0;

    private:

        std::unique_ptr<JsonHelper> mJsonHelper;
    };

    class LITE3DPP_EXPORT NoncopiableResource
    {
    public:

        NoncopiableResource();
    private:

        NoncopiableResource(const NoncopiableResource &other);
    };
}

