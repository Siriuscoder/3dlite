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

#include <lite3d/lite3d_texture_unit.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Texture : public ConfigurableResource, public Noncopiable
    {
    public:

        typedef stl<PixelsData>::vector LayersData;

        ~Texture();

        LITE3D_DECLARE_PTR_METHODS(lite3d_texture_unit, mTexture)

        inline bool isTextureBuffer()
        { return mTexture.isTextureBuffer == LITE3D_TRUE; }

        size_t usedVideoMemBytes() const override;

    protected:

        Texture(const String &name, 
            const String &path, Main *main);

        virtual void unloadImpl() override;

        lite3d_texture_unit mTexture = {0};
    };

    class LITE3DPP_EXPORT TextureImage : public Texture
    {
    public:

        TextureImage(const String &name, 
            const String &path, Main *main);

        ~TextureImage();

        inline int8_t getLevelsNum()
        { return mTexture.generatedMipmaps; }

        void getPixels(int8_t level, PixelsData &pixels);
        void getPixels(int8_t level, void *pixels);

        void setPixels(int8_t level, const PixelsData &pixels);
        void setPixels(int8_t level, const void *pixels);

        void getCompressedPixels(int8_t level, PixelsData &pixels);
        void getCompressedPixels(int8_t level, void *pixels);
        void setCompressedPixels(int8_t level, const PixelsData &pixels);
        void setCompressedPixels(int8_t level, const void *pixels, size_t size);

        size_t getLayerSize(int8_t level);
        size_t getCompressedLayerSize(int8_t level);
        
        /* commonly color.x = r, color.y = b ... */
        void setBlankColor(const kmVec4 &color);

        void generateMipmaps();
        
        inline int32_t getHeight()
        { return mTexture.imageHeight; }
        inline int32_t getWidth()
        { return mTexture.imageWidth; }
        inline int32_t getDepth()
        { return mTexture.imageDepth; }

        static uint32_t textureType(const String &s);
        static uint8_t textureFiltering(const String &s);
        static uint8_t textureWrap(const String &s);
        static uint32_t textureImageFormat(const String &s);
        static uint32_t textureFilterType(const String &s);
        static uint16_t textureFormat(const String &s);
        static uint16_t textureInternalFormat(int iformat);

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        bool mModifyed;
        LayersData mLayersBackup;
    };
}

