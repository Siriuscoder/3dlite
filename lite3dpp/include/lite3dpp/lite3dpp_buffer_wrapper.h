/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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

#include <lite3d/lite3d_mesh.h>
#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    using PixelsData = stl<uint8_t>::vector;
    using BufferData = PixelsData;
    using BufferLayout = stl<lite3d_vao_layout>::vector; 
    using MeshChunkArray = stl<lite3d_mesh_chunk *>::vector;

    class VertexArrayWrap
    {
    public:

        VertexArrayWrap(const BufferData &ptr, size_t elementsCount) : 
            mPtr(ptr.data()),
            mElementsCount(elementsCount)
        {}

        template <class T, typename std::enable_if_t<std::is_class_v<T>, bool> = true>
        VertexArrayWrap(const T &v) : 
            mPtr(v.data()),
            mElementsCount(v.size())
        {}

        template<class T>
        VertexArrayWrap(const T *ptr, size_t elementsCount) : 
            mPtr(ptr),
            mElementsCount(elementsCount)
        {}

        template<class T>
        inline const T *get() const
        {
            return static_cast<const T *>(mPtr);
        }

        inline size_t size() const
        {
            return mElementsCount;
        }

    private:

        const void *mPtr;
        size_t mElementsCount;
    };

    using IndexArrayWrap = VertexArrayWrap;
}
