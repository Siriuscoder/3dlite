/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    class BufferWrap
    {
    public:

        template <class T, typename std::enable_if_t<std::is_class_v<T>, bool> = true>
        BufferWrap(const T &v) : 
            mPtr(v.data()),
            mSize(v.size())
        {}

        template<class T>
        BufferWrap(const T *ptr, size_t size) : 
            mPtr(ptr),
            mSize(size)
        {}

        template<class T>
        inline const T *get() const
        {
            return static_cast<const T *>(mPtr);
        }

        inline size_t size() const
        {
            return mSize;
        }

    private:

        const void *mPtr;
        size_t mSize;
    };
}