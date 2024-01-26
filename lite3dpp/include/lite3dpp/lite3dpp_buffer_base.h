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
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_buffer_mapper.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT BufferBase
    {
    public:

        virtual ~BufferBase();

        virtual size_t bufferSizeBytes() const = 0;
        virtual void extendBufferBytes(size_t addsize) = 0;
        virtual void setBufferSizeBytes(size_t size) = 0;
        virtual void setData(const void *buffer, size_t offset, size_t size) = 0;
        virtual void getData(void *buffer, size_t offset, size_t size) const = 0;
        virtual void getData(BufferData &buffer, size_t offset, size_t size) const;
        virtual void setData(const BufferData &buffer, size_t offset);
        virtual BufferScopedMapper map(BufferScopedMapper::BufferScopedMapperLockType lockType) = 0;

        /* type align must be related with texel size */
        template<class T>
        T *getElement(uint32_t index, T *elem)
        {
            size_t offset = index * sizeof(T);
            getData(elem, offset, sizeof(T));
            return elem;
        }

        /* type align must be related with texel size */
        template<class T>
        void setElement(uint32_t index, const T *elem)
        {
            size_t offset = index * sizeof(T);
            setData(elem, offset, sizeof(T));
        }
    };
}
