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
#pragma once

#include <3dlite/3dlite_resource_pack.h>

#include <3dlitepp/3dlitepp_manageable.h>
#include <3dlitepp/3dlitepp_resource.h>

namespace lite3dpp
{
    template<class T>
    class LITE3DPP_EXPORT AbstractResourceManager : public Manageable
    {
    public:
        
        virtual T *loadResourceFromFile(const lite3dpp_string &fileName) = 0;
        virtual void unloadResource(T *resource) = 0;
        virtual void unloadResource(const lite3dpp_string &resourceName) = 0;
        virtual void unloadAllResources() = 0;
        virtual size_t loadedResourcesSize() const = 0;
        virtual void init() = 0;
        virtual void shut() = 0;
    };

    class LITE3DPP_EXPORT ResourceManager : public Manageable
    {
    public:

        typedef stl<lite3dpp_string, AbstractResource *>::map Resources;
        typedef stl<lite3dpp_string, lite3d_resource_pack*>::map Packs;

        typedef struct ResourceManagerStats
        {
            size_t resourcesHeapSize;
            size_t resourcesMappedSize;
            size_t totalFileCacheSize;
        } ResourceManagerStats;

        template<class T>
        T *queryResource(lite3dpp_string name, 
            const lite3dpp_string &path = "")
        {
            T *result = NULL;
            AbstractResource *resource;

            if((resource = fetchResource(name)) != NULL)
            {
                if((result = dynamic_cast<T*>(resource)) == NULL)
                    throw std::runtime_error((lite3dpp_string("Resource type mismatch: ") + 
                        name).c_str());
                return result;
            }

            if(path.size() == 0)
            {
                throw std::runtime_error((lite3dpp_string("Resource not found: ") + 
                    name).c_str());
            }

            if(name.size() == 0)
                name = generateName();

            try
            {
                /* resource not found.. create one */
                result = new T(name, mMain);
                loadResource(name, path, result);
            }
            catch(std::exception &ex)
            {
                delete result;
                throw ex;
            }

            return result;
        }

        ResourceManager(Main *main);
        virtual ~ResourceManager();

        void releaseAllResources();
        void releaseResource(const lite3dpp_string &name);

        ResourceManagerStats getStats() const;

        const void *loadFileToMemory(const lite3dpp_string &path, size_t *size);

        void addResourceLocation(const lite3dpp_string &name,
            const lite3dpp_string &path,
            size_t fileCacheMaxSize);

    protected:

        lite3dpp_string generateName();
        AbstractResource *fetchResource(const lite3dpp_string &key);
        virtual void loadResource(const lite3dpp_string &name, 
            const lite3dpp_string &path,
            AbstractResource *resource);
        virtual void mapResource(AbstractResource *resource);

    private:

        Main *mMain;
        Resources mResources;
        Packs mPacks;
    };
}

