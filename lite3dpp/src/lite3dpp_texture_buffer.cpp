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

    TextureBuffer::~TextureBuffer()
    {}

    void TextureBuffer::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (helper.getString(L"BufferFormat").size() > 0)
        {
            uint16_t tbf = 0;
            String tbfString = helper.getString(L"BufferFormat");

            if (tbfString == "R8")
                tbf = LITE3D_TB_R8;
            else if (tbfString == "R16")
                tbf = LITE3D_TB_R16;
            else if (tbfString == "R16F")
                tbf = LITE3D_TB_R16F;
            else if (tbfString == "R32F")
                tbf = LITE3D_TB_R32F;
            else if (tbfString == "R8I")
                tbf = LITE3D_TB_R8I;
            else if (tbfString == "R16I")
                tbf = LITE3D_TB_R16I;
            else if (tbfString == "R32I")
                tbf = LITE3D_TB_R32I;
            else if (tbfString == "R8UI")
                tbf = LITE3D_TB_R8UI;
            else if (tbfString == "R16UI")
                tbf = LITE3D_TB_R16UI;
            else if (tbfString == "R32UI")
                tbf = LITE3D_TB_R32UI;
            else if (tbfString == "RG8")
                tbf = LITE3D_TB_RG8;
            else if (tbfString == "RG16")
                tbf = LITE3D_TB_RG16;
            else if (tbfString == "RG16F")
                tbf = LITE3D_TB_RG16F;
            else if (tbfString == "RG32F")
                tbf = LITE3D_TB_RG32F;
            else if (tbfString == "RG8I")
                tbf = LITE3D_TB_RG8I;
            else if (tbfString == "RG16I")
                tbf = LITE3D_TB_RG16I;
            else if (tbfString == "RG32I")
                tbf = LITE3D_TB_RG32I;
            else if (tbfString == "RG8UI")
                tbf = LITE3D_TB_RG8UI;
            else if (tbfString == "RG16UI")
                tbf = LITE3D_TB_RG16UI;
            else if (tbfString == "RG32UI")
                tbf = LITE3D_TB_RG32UI;
            else if (tbfString == "RGB32F")
                tbf = LITE3D_TB_RGB32F;
            else if (tbfString == "RGB32I")
                tbf = LITE3D_TB_RGB32I;
            else if (tbfString == "RGB32UI")
                tbf = LITE3D_TB_RGB32UI;
            else if (tbfString == "RGBA8")
                tbf = LITE3D_TB_RGBA8;
            else if (tbfString == "RGBA16")
                tbf = LITE3D_TB_RGBA16;
            else if (tbfString == "RGBA16F")
                tbf = LITE3D_TB_RGBA16F;
            else if (tbfString == "RGBA32F")
                tbf = LITE3D_TB_RGBA32F;
            else if (tbfString == "RGBA8I")
                tbf = LITE3D_TB_RGBA8I;
            else if (tbfString == "RGBA16I")
                tbf = LITE3D_TB_RGBA16I;
            else if (tbfString == "RGBA32I")
                tbf = LITE3D_TB_RGBA32I;
            else if (tbfString == "RGBA8UI")
                tbf = LITE3D_TB_RGBA8UI;
            else if (tbfString == "RGBA16UI")
                tbf = LITE3D_TB_RGBA16UI;
            else if (tbfString == "RGBA32UI")
                tbf = LITE3D_TB_RGBA32UI;
            
            if (!lite3d_texture_buffer_allocate(&mTexture, helper.getInt(L"TexelsCount", 0), NULL, 
                tbf, helper.getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
            {
                LITE3D_THROW(getName() << " texture buffer allocation failed, probably it is not supported");
            }
        }
        else
            LITE3D_THROW(getName() << " texture buffer bad parameters");

        mTexture.userdata = this;
        setUsedVideoMem(mTexture.totalSize);
    }

    void TextureBuffer::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        /* reload json content */
        loadFromConfigImpl(helper);

        /* restore content */
        if (mTexelsBackup.size() > 0)
        {
            if (textureBufferSize() < mTexelsBackup.size())
            {
                size_t texelsCount = (mTexelsBackup.size() - textureBufferSize()) / 
                    lite3d_texture_buffer_texel_size(mTexture.texFormat);
                extendTextureBuffer(texelsCount);
            }

            setData(mTexelsBackup, 0);
            mTexelsBackup.clear();
            setUsedVideoMem(mTexture.totalSize);
        }
    }

    void TextureBuffer::unloadImpl()
    {
        getData(mTexelsBackup, 0, textureBufferSize());
        Texture::unloadImpl();
    }

    size_t TextureBuffer::textureBufferSize()
    {
        if (getState() != AbstractResource::LOADED)
            LITE3D_THROW(getName() << " resource unavailable");
        
        return mTexture.totalSize;
    }
    
    size_t TextureBuffer::textureBufferTexelsCount()
    {
        if (getState() != AbstractResource::LOADED)
            LITE3D_THROW(getName() << " resource unavailable");
        
        return mTexture.totalSize / lite3d_texture_buffer_texel_size(mTexture.texFormat);
    }
    
    void TextureBuffer::extendTextureBuffer(size_t texelsCount)
    {
        if (getState() != AbstractResource::LOADED)
            LITE3D_THROW(getName() << " resource unavailable");

        if (!lite3d_texture_buffer_extend(&mTexture, texelsCount * lite3d_texture_buffer_texel_size(mTexture.texFormat),
            getJson().getBool(L"Dynamic", false) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
        {
            LITE3D_THROW(getName() << " texture buffer extend failed, probably it is not supported");
        }

        setUsedVideoMem(mTexture.totalSize);
    }

    void TextureBuffer::setData(const void *buffer, size_t offset, size_t size)
    {
        if (getState() != AbstractResource::LOADED)
            LITE3D_THROW(getName() << " resource unavailable");
        if (size > 0)
        {
            if ((offset + size) > textureBufferSize())
                LITE3D_THROW(getName() << " operation may cause buffer overflow");
            if (!lite3d_texture_buffer(&mTexture, buffer, offset, size))
                LITE3D_THROW(getName() << " operation failed");
        }
    }

    void TextureBuffer::setData(const PixelsData &buffer, size_t offset)
    {
        setData(&buffer[0], offset, buffer.size());
    }

    void TextureBuffer::getData(void *buffer, size_t offset, size_t size)
    {
        if (getState() != AbstractResource::LOADED)
            LITE3D_THROW(getName() << " resource unavailable");
        if ((offset + size) > textureBufferSize())
            LITE3D_THROW(getName() << " requested size too big");
        if (!lite3d_texture_buffer_get(&mTexture, buffer, offset, size))
            LITE3D_THROW(getName() << " operation failed");
    }

    void TextureBuffer::getData(PixelsData &buffer, size_t offset, size_t size)
    {
        if (size > 0)
        {
            buffer.resize(size);
            getData(&buffer[0], offset, size);
        }
    }
}

