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
#include <lite3dpp/lite3dpp_buffer_mapper.h>

namespace lite3dpp
{
    BufferScopedMapper::BufferScopedMapper(lite3d_vbo &source, BufferScopedMapperLockType lockType) :
        mSource(source)
    {
        if((mPtr = lite3d_vbo_map(&source, lockType)) == NULL)
            LITE3D_THROW("vertex buffer map failed..");
    }

    BufferScopedMapper::BufferScopedMapper(const BufferScopedMapper &other) :
        mSource(other.mSource),
        mPtr(other.mPtr)
    {}

    BufferScopedMapper::BufferScopedMapper(BufferScopedMapper &&other) : 
        mSource(other.mSource),
        mPtr(other.mPtr)
    {
        other.mPtr = NULL;
    }

    BufferScopedMapper::~BufferScopedMapper()
    {
        if(mPtr)
            lite3d_vbo_unmap(&mSource);
    }
}