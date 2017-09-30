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
#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_buffer_base.h>

namespace lite3dpp
{
    void BufferBase::setData(const BufferData &buffer, size_t offset)
    {
        setData(&buffer[0], offset, buffer.size());
    }

    void BufferBase::getData(BufferData &buffer, size_t offset, size_t size) const
    {
        if (size > 0)
        {
            buffer.resize(size);
            getData(&buffer[0], offset, size);
        }
    }

    BufferBase::~BufferBase()
    {}
}
