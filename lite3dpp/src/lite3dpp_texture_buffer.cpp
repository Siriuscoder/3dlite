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
#include <algorithm>
#include <cctype>

#include <SDL_log.h>

#include <lite3d/lite3d_tbo.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>

namespace lite3dpp
{
    TextureBuffer::TextureBuffer(const String &name, 
        const String &path, Main *main) : 
        Texture(name, path, main)
    {}

    void TextureBuffer::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (helper.getString(L"BufferFormat").size() > 0)
        {
            String tbfString = helper.getUpperString(L"BufferFormat");
            uint16_t format = Texture::textureInternalFormat(helper.getUpperString(L"BufferFormat"));
            
            if (!lite3d_texture_buffer_init(&mTexture, helper.getInt(L"TexelsCount", 0), NULL, 
                format, helper.getBool(L"Dynamic", false) ? BufferUsage::ModeDynamicDraw : BufferUsage::ModeStaticDraw))
            {
                LITE3D_THROW(getName() << ": texture buffer allocation failed, probably it is not supported");
            }
        }
        else
            LITE3D_THROW(getName() << ": texture buffer bad parameters");

        mTexture.userdata = this;
    }

    void TextureBuffer::setBufferSizeBytes(size_t size)
    {
        if (valid())
            LITE3D_THROW(getName() << ": resource unavailable");

        replaceData(nullptr, size);
    }

    void TextureBuffer::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        /* reload json content */
        loadFromConfigImpl(helper);

        /* restore content */
        if (mTexelsBackup.size() > 0)
        {
            if (bufferSizeBytes() < mTexelsBackup.size())
            {
                extendBufferBytes(mTexelsBackup.size() - bufferSizeBytes());
            }

            setDataBuffer(mTexelsBackup, 0);
            mTexelsBackup.clear();
        }
    }

    void TextureBuffer::unloadImpl()
    {
        getDataBuffer(mTexelsBackup, 0, bufferSizeBytes());
        lite3d_texture_buffer_purge(&mTexture);
    }
    
    uint8_t TextureBuffer::texelSize() const
    {
        return lite3d_texture_buffer_texel_size(mTexture.internalFormat);
    }

    size_t TextureBuffer::bufferSizeBytes() const
    {
        return mTexture.totalSize;
    }
    
    size_t TextureBuffer::bufferSizeTexels() const
    {
        if (texelSize() == 0)
            LITE3D_THROW(getName() << ": unsupported texture buffer format");
        
        return bufferSizeBytes() / texelSize();
    }
    
    void TextureBuffer::extendBufferBytes(size_t addSize)
    {
        if (!valid())
            LITE3D_THROW(getName() << ": resource unavailable");

        if (!lite3d_texture_buffer_extend(&mTexture, addSize))
        {
            LITE3D_THROW(getName() << ": texture buffer extend failed, probably operation is not supported");
        }
    }

    void TextureBuffer::setData(const void *buffer, size_t offset, size_t size)
    {
        if (!valid())
            LITE3D_THROW(getName() << ": resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW(getName() << ": operation may cause buffer overflow");
            if (!lite3d_texture_buffer(&mTexture, buffer, offset, size))
                LITE3D_THROW(getName() << ": operation failed");
        }
    }

    void TextureBuffer::getData(void *buffer, size_t offset, size_t size) const
    {
        if (!valid())
            LITE3D_THROW(getName() << ": resource unavailable");
        
        if (size > 0)
        {
            if ((offset + size) > bufferSizeBytes())
                LITE3D_THROW(getName() << ": requested size too big");
            if (!lite3d_texture_buffer_get(&mTexture, buffer, offset, size))
                LITE3D_THROW(getName() << ": failed to upload data to texture buffer");
        }
    }

    void TextureBuffer::replaceData(const void *buffer, size_t size)
    {
        if (!valid())
            LITE3D_THROW(getName() << ": resource unavailable");

        if (!lite3d_vbo_buffer_alloc(&mTexture.tbo, buffer, size))
            LITE3D_THROW(getName() << ": failed to replace texture buffer data, new size is " << size << " bytes");
    }

    bool TextureBuffer::valid() const
    {
        return mTexture.textureID > 0 && getState() == AbstractResource::LOADED;
    }

    BufferScopedMapper TextureBuffer::map(BufferScopedMapper::BufferScopedMapperLockType lockType)
    {
        if (!valid())
            LITE3D_THROW(getName() << " resource unavailable");
        
        BufferScopedMapper mapper(mTexture.tbo, lockType);
        /* move constructor is a great power! */
        return mapper;
    }

    size_t TextureBuffer::usedVideoMemBytes() const
    {
        return mTexture.tbo.size;
    }
}

