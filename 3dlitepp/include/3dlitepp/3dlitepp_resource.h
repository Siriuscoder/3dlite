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

#include <3dlitepp/3dlitepp_manageable.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT AbstractResource : public Manageable
    {
    public:

        enum ResourceState
        {
            /* just created */
            NEW,
            /* json parsed, loaded to RAM */
            LOADED_UNMAPPED,
            /* loaded to Video memory */ 
            LOADED_MAPPED
        };

        enum ResourceType
        {
            SCRIPT
        };

        AbstractResource(const lite3dpp_string &name, 
            Main *main);
        virtual ~AbstractResource();

        void load(const void *buffer, size_t size);
        void unload();
        void map();
        void unmap();

        /* allocated size in heap */
        inline size_t heapSize() const
        { return mHeapSize; }
        /* allocated size in video memory (when resource LOADED_MAPPED) */
        inline size_t mappedSize() const
        { return mMappedSize; }

        inline ResourceState getState() const
        { return mState; }
        inline const lite3dpp_string &getName() const
        { return mName; }

    protected:

        virtual void loadImpl(const void *buffer, size_t size) = 0;
        virtual void unloadImpl() = 0;
        virtual void mapImpl() = 0;
        virtual void unmapImpl() = 0;

    protected:

        ResourceState mState;
        ResourceType mType;
        lite3dpp_string mName;
        size_t mHeapSize;
        size_t mMappedSize;
        Main *mMain;
    };
}

