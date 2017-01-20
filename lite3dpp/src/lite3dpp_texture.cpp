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

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_texture.h>

namespace lite3dpp
{
    Texture::Texture(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::TEXTURE)
    {
        memset(&mTexture, 0, sizeof(mTexture));
    }

    Texture::~Texture()
    {}

    void Texture::unloadImpl()
    {
        lite3d_texture_unit_purge(&mTexture);
        setUsedVideoMem(0);
    }

    TextureImage::TextureImage(const String &name, 
        const String &path, Main *main) : 
        Texture(name, path, main),
        mModifyed(false)
    {}

    TextureImage::~TextureImage()
    {}

    void TextureImage::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_texture_unit_compression(helper.getBool(L"Compression", true) ? LITE3D_TRUE : LITE3D_FALSE);

        String textureTypeStr = helper.getString(L"TextureType", "2D");
        String filteringStr = helper.getString(L"Filtering", "None");
        String wrappingStr = helper.getString(L"Wrapping", "ClampToEdge");
        uint32_t textureType = textureTypeStr == "1D" ? LITE3D_TEXTURE_1D : 
            (textureTypeStr == "2D" ? LITE3D_TEXTURE_2D : LITE3D_TEXTURE_3D);
        uint8_t quality = filteringStr == "None" ? LITE3D_TEXTURE_QL_LOW : 
            (filteringStr == "Linear" ? LITE3D_TEXTURE_QL_MEDIUM : 
            (filteringStr == "Trilinear" ? LITE3D_TEXTURE_QL_NICEST : 0));
        uint8_t wrapping = wrappingStr == "ClampToEdge" ? LITE3D_TEXTURE_CLAMP_TO_EDGE : 
            (wrappingStr == "Repeat" ? LITE3D_TEXTURE_REPEAT : 0);

        /* load texture from image */
        if (helper.getString(L"Image").size() > 0)
        {
            String imageFormatStr = helper.getString(L"ImageFormat", "ANY");

            std::transform(imageFormatStr.begin(), imageFormatStr.end(), imageFormatStr.begin(), [](char a) -> char
            { return std::toupper(a); });
            uint32_t imageType = imageFormatStr == "BMP" ? LITE3D_IMAGE_BMP : 
                (imageFormatStr == "JPG" ? LITE3D_IMAGE_JPG : 
                (imageFormatStr == "PNG" ? LITE3D_IMAGE_PNG : 
                (imageFormatStr == "TGA" ? LITE3D_IMAGE_TGA : 
                (imageFormatStr == "TIF" ? LITE3D_IMAGE_TIF :
                (imageFormatStr == "GIF" ? LITE3D_IMAGE_GIF :
                (imageFormatStr == "DDS" ? LITE3D_IMAGE_DDS : 
                (imageFormatStr == "PSD" ? LITE3D_IMAGE_PSD : 
                (imageFormatStr == "HDR" ? LITE3D_IMAGE_HDR : 
                (imageFormatStr == "ANY" ? LITE3D_IMAGE_ANY : 0)))))))));

            lite3d_texture_technique_reset_filters();
            for(const ConfigurationReader &filterConfig : helper.getObjects(L"ProcessingFilters"))
            {
                lite3d_image_filter filter;
                String filterTypeStr = filterConfig.getString(L"Type");
                std::transform(filterTypeStr.begin(), filterTypeStr.end(), filterTypeStr.begin(), [](char a) -> char
                { return std::toupper(a); });
                filter.filterID = filterTypeStr == "ALIENIFY" ? LITE3D_ALIENIFY_FILTER :
                    (filterTypeStr == "BLURAVG" ? LITE3D_BLURAVG_FILTER :
                    (filterTypeStr == "BLURGAUSSIAN" ? LITE3D_BLURGAUSSIAN_FILTER :
                    (filterTypeStr == "CONTRAST" ? LITE3D_CONTRAST_FILTER : 
                    (filterTypeStr == "GAMMACORRECT" ? LITE3D_GAMMACORRECT_FILTER :
                    (filterTypeStr == "MIRROR" ? LITE3D_MIRROR_FILTER :
                    (filterTypeStr == "NEGATIVE" ? LITE3D_NEGATIVE_FILTER : 
                    (filterTypeStr == "NOISIFY" ? LITE3D_NOISIFY_FILTER :
                    (filterTypeStr == "PIXELIZE" ? LITE3D_PIXELIZE_FILTER :
                    (filterTypeStr == "WAVE" ? LITE3D_WAVE_FILTER :
                    (filterTypeStr == "SHARPEN" ? LITE3D_SHARPEN_FILTER : 0))))))))));

                filter.param1 = (float)filterConfig.getDouble(L"Param1");
                filter.param2 = (float)filterConfig.getDouble(L"Param2");
                lite3d_texture_technique_add_image_filter(&filter);
            }

            if(!lite3d_texture_unit_from_resource(&mTexture, mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Image")),
                imageType, textureType, quality, wrapping))
                LITE3D_THROW(getName() << " texture load failed..");
        }
        else
        {
            String textureFormatStr = helper.getString(L"TextureFormat", "RGB");
            std::transform(textureFormatStr.begin(), textureFormatStr.end(), textureFormatStr.begin(), [](char a) -> char
            { return std::toupper(a); });
            uint16_t textureFormat = textureFormatStr == "ALPHA" ? LITE3D_TEXTURE_FORMAT_ALPHA :
                (textureFormatStr == "RGB" ? LITE3D_TEXTURE_FORMAT_RGB : 
                (textureFormatStr == "RGBA" ? LITE3D_TEXTURE_FORMAT_RGBA : 
                (textureFormatStr == "BRG" ? LITE3D_TEXTURE_FORMAT_BRG : 
                (textureFormatStr == "BRGA" ? LITE3D_TEXTURE_FORMAT_BRGA : 
                (textureFormatStr == "LUMINANCE" ? LITE3D_TEXTURE_FORMAT_LUMINANCE : 
                (textureFormatStr == "LUMINANCE_ALPHA" ? LITE3D_TEXTURE_FORMAT_LUMINANCE_ALPHA :
                (textureFormatStr == "DEPTH" ? LITE3D_TEXTURE_FORMAT_DEPTH : 0)))))));

            if(!lite3d_texture_unit_allocate(&mTexture, textureType, quality, wrapping, textureFormat, helper.getInt(L"InternalFormat", 0),
                helper.getInt(L"Height", 1), helper.getInt(L"Width", 1), helper.getInt(L"Depth", 1)))
                LITE3D_THROW(getName() << " texture allocation failed..");
            
            if(helper.has(L"BlankColor"))
                setBlankColor(helper.getVec4(L"BlankColor"));
        }

        mTexture.userdata = this;
        setUsedVideoMem(mTexture.totalSize);
    }

    void TextureImage::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        /* reload json content */
        loadFromConfigImpl(helper);

        /* restore modifyed content */
        if(mModifyed)
        {
            for(int8_t level = 0; level <= getLevelsNum() && level < (int8_t)mLayersBackup.size(); ++level)
            {
                if(mTexture.compressed)
                    setCompressedPixels(level, mLayersBackup[level]);
                else
                    setPixels(level, mLayersBackup[level]);
            }

            mLayersBackup.clear();
        }
    }

    void TextureImage::unloadImpl()
    {
        if(mModifyed)
        {
            for(int8_t level = 0; level <= getLevelsNum(); ++level)
            {
                PixelsData pixels;

                if(mTexture.compressed)
                    getCompressedPixels(level, pixels);
                else
                    getPixels(level, pixels);

                mLayersBackup.push_back(std::move(pixels));
            }
        }

        Texture::unloadImpl();
    }

    void TextureImage::getPixels(int8_t level, PixelsData &pixels)
    {
        size_t size;
        if(!lite3d_texture_unit_get_level_size(&mTexture, level, &size))
            LITE3D_THROW("Could`n get size of level " << level << " for texture " << getName());

        pixels.resize(size);
        if(!lite3d_texture_unit_get_pixels(&mTexture, level, &pixels[0]))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::getPixels(int8_t level, void *pixels)
    {
        if(!lite3d_texture_unit_get_pixels(&mTexture, level, pixels))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::setPixels(int8_t level, const PixelsData &pixels)
    {
        setPixels(level, &pixels[0]);
    }

    void TextureImage::setPixels(int8_t level, const void *pixels)
    {
        if(!lite3d_texture_unit_set_pixels(&mTexture, 0, 0, 0,
            lite3d_texture_unit_get_level_width(&mTexture, level), 
            lite3d_texture_unit_get_level_height(&mTexture, level), 
            lite3d_texture_unit_get_level_depth(&mTexture, level), 
            level, pixels))
            LITE3D_THROW("Could`n set level " << level << " for texture ");

        mModifyed = true;
    }

    void TextureImage::getCompressedPixels(int8_t level, PixelsData &pixels)
    {
        size_t size;
        if(!lite3d_texture_unit_get_compressed_level_size(&mTexture, level, &size))
            LITE3D_THROW("Could`n get size of level " << level << " for texture " << getName());

        pixels.resize(size);
        if(!lite3d_texture_unit_get_compressed_pixels(&mTexture, level, &pixels[0]))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::getCompressedPixels(int8_t level, void *pixels)
    {
        if(!lite3d_texture_unit_get_compressed_pixels(&mTexture, level, pixels))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::setCompressedPixels(int8_t level, const PixelsData &pixels)
    {
        setCompressedPixels(level, &pixels[0], pixels.size());
    }

    void TextureImage::setCompressedPixels(int8_t level, const void *pixels, size_t size)
    {
        if(!lite3d_texture_unit_set_compressed_pixels(&mTexture, 0, 0, 0,
            lite3d_texture_unit_get_level_width(&mTexture, level), 
            lite3d_texture_unit_get_level_height(&mTexture, level), 
            lite3d_texture_unit_get_level_depth(&mTexture, level), 
            level, size, pixels))
            LITE3D_THROW("Could`n set level " << level << " for texture ");

        mModifyed = true;
    }

    void TextureImage::generateMipmaps()
    {
        lite3d_texture_unit_generate_mipmaps(&mTexture);
    }

    size_t TextureImage::getLayerSize(int8_t level)
    {
        size_t res;
        if(!lite3d_texture_unit_get_level_size(&mTexture, level, &res))
            LITE3D_THROW("Could`n dump level " << level << " size for texture " << getName());

        return res;
    }

    size_t TextureImage::getCompressedLayerSize(int8_t level)
    {
        size_t res;
        if(!lite3d_texture_unit_get_compressed_level_size(&mTexture, level, &res))
            LITE3D_THROW("Could`n dump level " << level << " size for texture " << getName());

        return res;
    }
    
    void TextureImage::setBlankColor(const kmVec4 &color)
    {
        /* fullup pixels */
        PixelsData pixels(mTexture.imageSize);
        for(unsigned j = 0; j < pixels.size();)
        {
            if(mTexture.imageBPP >= 1)
                pixels[j++] = color.x * 255;
            if(mTexture.imageBPP >= 2)
                pixels[j++] = color.y * 255;
            if(mTexture.imageBPP >= 3)
                pixels[j++] = color.z * 255;
            if(mTexture.imageBPP == 4)
                pixels[j++] = color.w * 255;
        }
        
        /* upload pixels */
        setPixels(0, pixels);
        generateMipmaps();
    }
}

