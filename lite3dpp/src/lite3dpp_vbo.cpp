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
#include <lite3dpp/lite3dpp_vbo.h>

namespace lite3dpp
{
    VBO::VBO(lite3d_vbo &vbo) : 
        mVBO(vbo)
    {}

    size_t VBO::bufferSizeBytes() const
    {
        return mVBO.size;
    }
    
    void VBO::extendBufferBytes(size_t addSize)
    {
        if (!mVBO.vboID)
            LITE3D_THROW("resource unavailable");

        if (!lite3d_vbo_extend(&mVBO, addSize, mVBO.access))
        {
            LITE3D_THROW("vertex buffer extend failed, probably it is not supported");
        }
    }

    void VBO::setData(const void *buffer, size_t offset, size_t size)
    {
        if (!mVBO.vboID)
            LITE3D_THROW("resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW("operation may cause buffer overflow");
            if (!lite3d_vbo_subbuffer(&mVBO, buffer, offset, size))
                LITE3D_THROW("failed to upload data to vertex buffer");
        }
    }

    void VBO::getData(void *buffer, size_t offset, size_t size) const
    {
        if (!mVBO.vboID)
            LITE3D_THROW("resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW("vertex buffer read out of bounds: " << (offset + size) << " of " << bufferSizeBytes());
            if (!lite3d_vbo_get_buffer(&mVBO, buffer, offset, size))
                LITE3D_THROW("failed to read data from vertex buffer");
        }
    }
    
    BufferScopedMapper VBO::map(uint16_t lockType)
    {
        if (!mVBO.vboID)
            LITE3D_THROW("resource unavailable");
        
        BufferScopedMapper mapper(mVBO, lockType);
        /* move constructor is a great power! */
        return mapper;
    }
}

