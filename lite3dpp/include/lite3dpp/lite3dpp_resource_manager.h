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

#include <lite3d/lite3d_pack.h>

#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ResourceManager : public Manageable
    {
    public:

        typedef stl<String, AbstractResource *>::map Resources;
        typedef stl<String, lite3d_pack*>::map Packs;

        typedef struct ResourceManagerStats
        {
            /* total allocated video memory buffers size */
            size_t bufferedSize;
            size_t totalFileCacheSize;
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
                    throw std::runtime_error((String("Resource type mismatch: ") + 
                        name).c_str());
                return result;
            }

            if(path.size() == 0)
            {
                throw std::runtime_error((String("Resource not found: ") + 
                    name).c_str());
            }

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
            AbstractResource *resource;

            if((resource = fetchResource(name)) != NULL)
            {
                T *result;
                if((result = dynamic_cast<T*>(resource)) == NULL)
                    throw std::runtime_error((String("Resource type mismatch: ") + 
                        name).c_str());
                return result;
            }

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
        
        template<class T>
        T *queryResource(String name, 
            const ResourceParameters &params)
        {
            AbstractResource *resource;

            if((resource = fetchResource(name)) != NULL)
            {
                T *result;
                if((result = dynamic_cast<T*>(resource)) == NULL)
                    throw std::runtime_error((String("Resource type mismatch: ") + 
                        name).c_str());
                return result;
            }

            if(name.size() == 0)
                name = generateResourceName();

            /* resource not found.. create one */
            std::unique_ptr<T> result(new T(name, "", mMain));
            loadResource(name, params, result.get());

            return result.release();
        }
        
        template<class T>
        T *queryResource(const ResourceParameters &params)
        {
            return queryResource<T>("", params);
        }

        ResourceManager(Main *main);
        virtual ~ResourceManager();

        void releaseAllResources();
        void releaseResource(const String &name);
        void releaseFileCache();
        
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
        virtual void loadResource(const String &name, 
            const ResourceParameters &params,
            AbstractResource *resource);

    private:

        Main *mMain;
        Resources mResources;
        Packs mPacks;
    };
}

