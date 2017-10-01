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
#pragma once

#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_config_reader.h>

#define LITE3D_EMPTY_JSON                       "{}\0"

namespace lite3dpp
{
    typedef stl<uint8_t>::vector PixelsData, BufferData;
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
            RENDER_TARGET,
            SHADER_STORAGE
        };

        AbstractResource(const String &name, 
            const String &path, Main *main, ResourceType type);
        virtual ~AbstractResource();

        void load(const void *buffer, size_t size);
        /* call this to reload object */ 
        void reload();
        void unload();

        /* allocated size in video memory (when resource LOADED_MAPPED) */
        virtual size_t usedVideoMemBytes() const;
        virtual size_t fullVideoMemBytes() const;

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
        virtual void reloadImpl() = 0;
        virtual void unloadImpl() = 0;

    private:
        
        void logState();

        ResourceState mState;
        ResourceType mType;
        String mName;
        String mPath;

    protected:
        Main *mMain;
    };

    class LITE3DPP_EXPORT ConfigurableResource : public AbstractResource
    {
    public:

        static const String emptyJson;

        ConfigurableResource(const String &name, 
            const String &path, Main *main, ResourceType type);
        virtual ~ConfigurableResource();

        const ConfigurationReader &getJson() const;

    protected:

        virtual void loadImpl(const void *buffer, size_t size) override final;
        virtual void reloadImpl() override final;
        virtual void loadFromConfigImpl(const ConfigurationReader &helper) = 0;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper);
    private:

        std::unique_ptr<ConfigurationReader> mConfiguration;
    };
}

