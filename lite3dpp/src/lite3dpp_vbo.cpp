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

    bool VBO::valid() const 
    {
        return mVBO.vboID != 0;
    }
    
    void VBO::extendBufferBytes(size_t addSize)
    {
        if (!valid())
            LITE3D_THROW("Failed to extend GPU buffer, resource unavailable");

        if (!lite3d_vbo_extend(&mVBO, addSize))
        {
            LITE3D_THROW("Failed to extend GPU buffer, probably this function does not supported");
        }
    }

    void VBO::setBufferSizeBytes(size_t size)
    {
        if (!valid())
            LITE3D_THROW("Failed to resize GPU buffer, resource unavailable");

        replaceData(nullptr, 0);
    }

    void VBO::setData(const void *buffer, size_t offset, size_t size)
    {
        if (!valid())
            LITE3D_THROW("Failed to upload data to GPU buffer, resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW("operation may cause GPU buffer overflow");
            if (!lite3d_vbo_subbuffer(&mVBO, buffer, offset, size))
                LITE3D_THROW("Failed to upload data to GPU");
        }
    }

    void VBO::getData(void *buffer, size_t offset, size_t size) const
    {
        if (!valid())
            LITE3D_THROW("Failed to fetch data from GPU buffer, resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW("GPU buffer read out of bounds: " << (offset + size) << " of " << bufferSizeBytes());
            if (!lite3d_vbo_get_buffer(&mVBO, buffer, offset, size))
                LITE3D_THROW("Failed to read data from GPU buffer");
        }
    }

    void VBO::replaceData(const void *buffer, size_t size)
    {
        if (!valid())
            LITE3D_THROW("Failed to upload data to GPU buffer, resource unavailable");

        if (!lite3d_vbo_buffer(&mVBO, buffer, size))
            LITE3D_THROW("Failed to upload data to GPU buffer");
    }
    
    BufferScopedMapper VBO::map(BufferScopedMapper::BufferScopedMapperLockType lockType)
    {
        if (!valid())
            LITE3D_THROW("Failed to map GPU buffer, resource unavailable");
        
        BufferScopedMapper mapper(mVBO, lockType);
        /* move constructor is a great power! */
        return mapper;
    }
    
    VBOResource::VBOResource(const String &name, const String &path, Main *main,
        AbstractResource::ResourceType type) : 
        ConfigurableResource(name, path, main, type),
        VBO(mVBO)
    {
        mVBO.userdata = this;
    }
    
    VBOResource::~VBOResource()
    {
        lite3d_vbo_purge(&mVBO);
    }
    
    size_t VBOResource::usedVideoMemBytes() const
    {
        return mVBO.size;
    }
    
    void VBOResource::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        size_t size;
        if ((size = helper.getInt(L"Size", 0)) > 0)
        {
            setBufferSizeBytes(size);
        }
    }
    
    void VBOResource::unloadImpl()
    {
        if (bufferSizeBytes() > 0)
        {
            /* load data into host memory */
            getDataBuffer(mVBOData, 0, bufferSizeBytes());
            setBufferSizeBytes(0);
        }
    }
    
    void VBOResource::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (mVBOData.size() > 0)
        {
            replaceDataBuffer(mVBOData);
        }
    }
}

