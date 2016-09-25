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

#include <lite3d/lite3d_pack.h>

#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ResourceManager : public Manageable, public Noncopiable
    {
    public:

        typedef stl<String, AbstractResource *>::map Resources;
        typedef stl<String, lite3d_pack*>::map Packs;

        typedef struct ResourceManagerStats
        {
            /* total allocated video memory buffers size */
            size_t usedVideoMem;
            uint32_t totalObjectsCount;
            uint32_t texturesCount;
            uint32_t texturesLoadedCount;
            uint32_t materialsCount;
            uint32_t materialsLoadedCount;
            uint32_t scriptsCount;
            uint32_t scriptsLoadedCount;
            uint32_t meshesCount;
            uint32_t meshesLoadedCount;
            uint32_t scenesCount;
            uint32_t scenesLoadedCount;
            uint32_t shaderProgramsCount;
            uint32_t shaderProgramsLoadedCount;
            uint32_t renderTargetsCount;
            uint32_t renderTargetsLoadedCount;
            uint32_t fileCachesCount;
            size_t totalCachedFilesMemSize;
        } ResourceManagerStats;

        template<class T>
        T *queryResource(String name,
            const String &path = "")
        {
            AbstractResource *resource;

            if((resource = fetchResource(name)) != NULL)
            {
                T *result;
                if((result = dynamic_cast<T*>(resource)) == NULL)
                    LITE3D_THROW("Resource type mismatch: " << name);
                return result;
            }

            if(path.size() == 0)
                LITE3D_THROW("Resource not found: " << name);

            if(name.size() == 0)
                name = generateResourceName();

            /* resource not found.. create one */
            std::unique_ptr<T> result(new T(name, path, mMain));
            loadResource(name, path, result.get());

            return result.release();
        }
        
        template<class T>
        T *queryResource(String name, 
            const void *data, size_t size)
        {
            if(name.size() == 0)
                name = generateResourceName();

            /* resource not found.. create one */
            std::unique_ptr<T> result(new T(name, "", mMain));
            loadResource(name, data, size, result.get());

            return result.release();
        }
        
        template<class T>
        T *queryResource(const void *data, size_t size)
        {
            return queryResource<T>("", data, size);
        }

        ResourceManager(Main *main);
        virtual ~ResourceManager();

        void releaseAllResources();
        void releaseResource(const String &name);
        void releaseFileCache();
        void releaseFileCache(const String &location);

        
        ResourceManagerStats getStats() const;

        const void *loadFileToMemory(const String &path, size_t *size);
        const lite3d_file *loadFileToMemory(const String &path);

        void addResourceLocation(const String &name,
            const String &path,
            size_t fileCacheMaxSize);

    protected:

        String generateResourceName();
        AbstractResource *fetchResource(const String &key);
        virtual void loadResource(const String &name, 
            const String &path,
            AbstractResource *resource);
        virtual void loadResource(const String &name, 
            const void *buffer, size_t size,
            AbstractResource *resource);

    private:

        Main *mMain;
        Resources mResources;
        Packs mPacks;
        lite3d_pack *mLastUsed;
    };
}

