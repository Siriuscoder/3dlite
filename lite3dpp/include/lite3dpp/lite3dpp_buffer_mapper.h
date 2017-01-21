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

#include <lite3d/lite3d_vbo.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT BufferScopedMapper : public Manageable, public Noncopiable
    {
    private:
        
        BufferScopedMapper(const BufferScopedMapper &other);
        
    public:
        BufferScopedMapper(lite3d_vbo &source, uint16_t lockType);
        BufferScopedMapper(BufferScopedMapper &&other);
        ~BufferScopedMapper();

        template<class T>
        const T *getPtr() const
        { return static_cast<T *>(mPtr); }

        template<class T>
        T *getPtr()
        { return static_cast<T *>(mPtr); }

        inline size_t getSize()
        { return mSource.size; }

    private:

        lite3d_vbo &mSource;
        void *mPtr;
    };
}

