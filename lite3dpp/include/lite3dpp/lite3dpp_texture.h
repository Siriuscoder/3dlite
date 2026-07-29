/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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

        virtual ~Texture() = default;

        LITE3D_DECLARE_PTR_METHODS(lite3d_texture_unit, mTexture)

        inline bool isTextureBuffer()
        { return mTexture.isTextureBuffer == LITE3D_TRUE; }

        size_t usedVideoMemBytes() const override;

        uint64_t handle();

    protected:

        Texture(const String &name, 
            const String &path, Main &main);

        virtual void unloadImpl() override;

        static uint32_t textureType(const String &s);
        static uint8_t textureFiltering(const String &s);
        static uint8_t textureWrap(const String &s);
        static uint32_t textureImageFormat(const String &s);
        static uint32_t textureFilterType(const String &s);
        static uint16_t textureFormat(const String &s);
        static uint16_t textureInternalFormat(const String& s);

    protected:

        lite3d_texture_unit mTexture = {0};
    };

    class LITE3DPP_EXPORT TextureImage : public Texture
    {
    public:

        TextureImage(const String &name, 
            const String &path, Main &main);

        virtual ~TextureImage() = default;

        /**
         * @return The total number of mipmap levels in the texture.
         * One means no mipmaps have been generated/loaded, but a single level exists.
         */
        inline int8_t getTotalLevels() const
        { return mTexture.generatedMipmaps + 1; }

        void getPixels(int8_t level, PixelsData &pixels) const;
        void getPixels(int8_t level, PixelsFloatData &pixels) const;
        void getPixels(int8_t level, void *pixels) const;
        void getPixels(int8_t level, float *pixels) const;

        void setPixels(int8_t level, const PixelsData &pixels);
        void setPixels(int8_t level, const PixelsFloatData &pixels);
        void setPixels(int8_t level, const uint8_t *pixels);
        void setPixels(int8_t level, const float *pixels);

        void getCompressedPixels(int8_t level, PixelsData &pixels) const;
        void getCompressedPixels(int8_t level, void *pixels) const;
        void setCompressedPixels(int8_t level, const PixelsData &pixels);
        void setCompressedPixels(int8_t level, const void *pixels, size_t size);

        size_t getLevelSize(int8_t level) const;
        size_t getCompressedLevelSize(int8_t level) const;
        
        /* commonly color.x = r, color.y = b ... */
        void setBlankColor(const kmVec4 &color);

        void generateMipmaps();

        void copyFrom(const TextureImage &srcTex, int8_t level = 0);
        void copyRegionFrom(const TextureImage &dstTex,  
            int32_t srcWidthOff, int32_t srcHeightOff, int32_t srcDepthOff,
            int32_t dstWidthOff, int32_t dstHeightOff, int32_t dstDepthOff,
            int32_t width, int32_t height, int32_t depth, int8_t level = 0);
        
        inline int32_t getHeight() const
        { return mTexture.imageHeight; }
        inline int32_t getWidth() const
        { return mTexture.imageWidth; }
        inline int32_t getDepth() const
        { return mTexture.imageDepth; }
        inline int32_t getFaceLayerDepth() const
        {
            if (mTexture.textureTarget == LITE3D_TEXTURE_CUBE_ARRAY)
                return mTexture.imageDepth * 6;
             
            return mTexture.imageDepth; 
        }

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        bool mModified;
    };
}

