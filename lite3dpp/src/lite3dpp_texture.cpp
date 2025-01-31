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

    void Texture::unloadImpl()
    {
        lite3d_texture_unit_purge(&mTexture);
    }

    size_t Texture::usedVideoMemBytes() const
    {
        return mTexture.totalSize;
    }

    uint32_t Texture::textureType(const String &s)
    {
        return s == "1D" ? LITE3D_TEXTURE_1D :
            (s == "2D" ? LITE3D_TEXTURE_2D :
            (s == "2D_ARRAY" ? LITE3D_TEXTURE_2D_ARRAY :
            (s == "2D_SHADOW" ? LITE3D_TEXTURE_2D_SHADOW :
            (s == "2D_SHADOW_ARRAY" ? LITE3D_TEXTURE_2D_SHADOW_ARRAY :
            (s == "CUBE" ? LITE3D_TEXTURE_CUBE :
            (s == "3D" ? LITE3D_TEXTURE_3D : 
            (s == "CUBE_ARRAY" ? LITE3D_TEXTURE_CUBE_ARRAY : 0xff)))))));
    }

    uint8_t Texture::textureFiltering(const String &s)
    {
        return s == "NONE" ? LITE3D_TEXTURE_FILTER_NEAREST :
            (s == "LINEAR" ? LITE3D_TEXTURE_FILTER_BILINEAR :
            (s == "TRILINEAR" ? LITE3D_TEXTURE_FILTER_TRILINEAR : 0));
    }

    uint8_t Texture::textureWrap(const String &s)
    {
        return s == "CLAMPTOEDGE" ? LITE3D_TEXTURE_CLAMP_TO_EDGE :
            (s == "REPEAT" ? LITE3D_TEXTURE_REPEAT : 0);
    }

    uint32_t Texture::textureImageFormat(const String &s)
    {
        return s == "BMP" ? LITE3D_IMAGE_BMP :
            (s == "JPG" ? LITE3D_IMAGE_JPG :
            (s == "PNG" ? LITE3D_IMAGE_PNG :
            (s == "TGA" ? LITE3D_IMAGE_TGA :
            (s == "TIF" ? LITE3D_IMAGE_TIF :
            (s == "GIF" ? LITE3D_IMAGE_GIF :
            (s == "DDS" ? LITE3D_IMAGE_DDS :
            (s == "PSD" ? LITE3D_IMAGE_PSD :
            (s == "HDR" ? LITE3D_IMAGE_HDR :
            (s == "ANY" ? LITE3D_IMAGE_ANY : 0)))))))));
    }

    uint32_t Texture::textureFilterType(const String &s)
    {
        return s == "ALIENIFY" ? LITE3D_ALIENIFY_FILTER :
            (s == "BLURAVG" ? LITE3D_BLURAVG_FILTER :
            (s == "BLURGAUSSIAN" ? LITE3D_BLURGAUSSIAN_FILTER :
            (s == "CONTRAST" ? LITE3D_CONTRAST_FILTER :
            (s == "GAMMACORRECT" ? LITE3D_GAMMACORRECT_FILTER :
            (s == "MIRROR" ? LITE3D_MIRROR_FILTER :
            (s == "FLIP" ? LITE3D_FLIP_FILTER :
            (s == "NEGATIVE" ? LITE3D_NEGATIVE_FILTER :
            (s == "NOISIFY" ? LITE3D_NOISIFY_FILTER :
            (s == "PIXELIZE" ? LITE3D_PIXELIZE_FILTER :
            (s == "WAVE" ? LITE3D_WAVE_FILTER :
            (s == "SHARPEN" ? LITE3D_SHARPEN_FILTER : 0)))))))))));
    }

    uint16_t Texture::textureFormat(const String &s)
    {
        return s == "RED" ? LITE3D_TEXTURE_FORMAT_RED :
            (s == "RGB" ? LITE3D_TEXTURE_FORMAT_RGB :
            (s == "RGBA" ? LITE3D_TEXTURE_FORMAT_RGBA :
            (s == "BRG" ? LITE3D_TEXTURE_FORMAT_BRG :
            (s == "BRGA" ? LITE3D_TEXTURE_FORMAT_BRGA :
            (s == "RG" ? LITE3D_TEXTURE_FORMAT_RG :
            (s == "DEPTH" ? LITE3D_TEXTURE_FORMAT_DEPTH : 0))))));
    }

    uint16_t Texture::textureInternalFormat(const String &internalFormat)
    {
        static const stl<String, uint16_t>::unordered_map internalFormats = {
            {"R8", LITE3D_TEXTURE_INTERNAL_R8},
            {"R8_SNORM", LITE3D_TEXTURE_INTERNAL_R8_SNORM},
            {"R16", LITE3D_TEXTURE_INTERNAL_R16},
            {"R16_SNORM", LITE3D_TEXTURE_INTERNAL_R16_SNORM},
            {"RG8", LITE3D_TEXTURE_INTERNAL_RG8},
            {"RG8_SNORM", LITE3D_TEXTURE_INTERNAL_RG8_SNORM},
            {"RG16", LITE3D_TEXTURE_INTERNAL_RG16},
            {"RG16_SNORM", LITE3D_TEXTURE_INTERNAL_RG16_SNORM},
            {"R3_G3_B2", LITE3D_TEXTURE_INTERNAL_R3_G3_B2},
            {"RGB4", LITE3D_TEXTURE_INTERNAL_RGB4},
            {"RGB5", LITE3D_TEXTURE_INTERNAL_RGB5},
            {"RGB8", LITE3D_TEXTURE_INTERNAL_RGB8},
            {"RGB8_SNORM", LITE3D_TEXTURE_INTERNAL_RGB8_SNORM},
            {"RGB10", LITE3D_TEXTURE_INTERNAL_RGB10},
            {"RGB12", LITE3D_TEXTURE_INTERNAL_RGB12},
            {"RGB16_SNORM", LITE3D_TEXTURE_INTERNAL_RGB16_SNORM},
            {"RGBA2", LITE3D_TEXTURE_INTERNAL_RGBA2},
            {"RGBA4", LITE3D_TEXTURE_INTERNAL_RGBA4},
            {"RGB5_A1", LITE3D_TEXTURE_INTERNAL_RGB5_A1},
            {"RGBA8", LITE3D_TEXTURE_INTERNAL_RGBA8},
            {"RGBA8_SNORM", LITE3D_TEXTURE_INTERNAL_RGBA8_SNORM},
            {"RGB10_A2", LITE3D_TEXTURE_INTERNAL_RGB10_A2},
            {"RGB10_A2UI", LITE3D_TEXTURE_INTERNAL_RGB10_A2UI},
            {"RGBA12", LITE3D_TEXTURE_INTERNAL_RGBA12},
            {"RGBA16", LITE3D_TEXTURE_INTERNAL_RGBA16},
            {"SRGB8", LITE3D_TEXTURE_INTERNAL_SRGB8},
            {"SRGB8_ALPHA8", LITE3D_TEXTURE_INTERNAL_SRGB8_ALPHA8},
            {"SRGB", LITE3D_TEXTURE_INTERNAL_SRGB},
            {"SRGB_ALPHA", LITE3D_TEXTURE_INTERNAL_SRGB_ALPHA},
            {"R16F", LITE3D_TEXTURE_INTERNAL_R16F},
            {"RG16F", LITE3D_TEXTURE_INTERNAL_RG16F},
            {"RGB16F", LITE3D_TEXTURE_INTERNAL_RGB16F},
            {"RGBA16F", LITE3D_TEXTURE_INTERNAL_RGBA16F},
            {"R32F", LITE3D_TEXTURE_INTERNAL_R32F},
            {"RG32F", LITE3D_TEXTURE_INTERNAL_RG32F},
            {"RGB32F", LITE3D_TEXTURE_INTERNAL_RGB32F},
            {"RGBA32F", LITE3D_TEXTURE_INTERNAL_RGBA32F},
            {"R11F_G11F_B10F", LITE3D_TEXTURE_INTERNAL_R11F_G11F_B10F},
            {"RGB9_E5", LITE3D_TEXTURE_INTERNAL_RGB9_E5},
            {"R8I", LITE3D_TEXTURE_INTERNAL_R8I},
            {"R8UI", LITE3D_TEXTURE_INTERNAL_R8UI},
            {"R16I", LITE3D_TEXTURE_INTERNAL_R16I},
            {"R16UI", LITE3D_TEXTURE_INTERNAL_R16UI},
            {"R32I", LITE3D_TEXTURE_INTERNAL_R32I},
            {"R32UI", LITE3D_TEXTURE_INTERNAL_R32UI},
            {"RG8I", LITE3D_TEXTURE_INTERNAL_RG8I},
            {"RG8UI", LITE3D_TEXTURE_INTERNAL_RG8UI},
            {"RG16I", LITE3D_TEXTURE_INTERNAL_RG16I},
            {"RG16UI", LITE3D_TEXTURE_INTERNAL_RG16UI},
            {"RG32I", LITE3D_TEXTURE_INTERNAL_RG32I},
            {"RG32UI", LITE3D_TEXTURE_INTERNAL_RG32UI},
            {"RGB8I", LITE3D_TEXTURE_INTERNAL_RGB8I},
            {"RGB8UI", LITE3D_TEXTURE_INTERNAL_RGB8UI},
            {"RGB16I", LITE3D_TEXTURE_INTERNAL_RGB16I},
            {"RGB16UI", LITE3D_TEXTURE_INTERNAL_RGB16UI},
            {"RGB32I", LITE3D_TEXTURE_INTERNAL_RGB32I},
            {"RGB32UI", LITE3D_TEXTURE_INTERNAL_RGB32UI},
            {"RGBA8I", LITE3D_TEXTURE_INTERNAL_RGBA8I},
            {"RGBA8UI", LITE3D_TEXTURE_INTERNAL_RGBA8UI},
            {"RGBA16I", LITE3D_TEXTURE_INTERNAL_RGBA16I},
            {"RGBA16UI", LITE3D_TEXTURE_INTERNAL_RGBA16UI},
            {"RGBA32I", LITE3D_TEXTURE_INTERNAL_RGBA32I},
            {"RGBA32UI", LITE3D_TEXTURE_INTERNAL_RGBA32UI}
        };

        if (internalFormat.empty())
            return 0;

        auto it = internalFormats.find(internalFormat);
        if (it != internalFormats.end())
        {
            return it->second;
        }

        LITE3D_THROW("Invalid Texture Internal Format: " << internalFormat);
    }

    uint64_t Texture::handle()
    {
        if (!lite3d_texture_unit_extract_handle(&mTexture))
        {
            LITE3D_THROW(getName() << ": Failed to extract texture handle");
        }

        return mTexture.handle;
    }

    TextureImage::TextureImage(const String &name, 
        const String &path, Main *main) : 
        Texture(name, path, main),
        mModifyed(false)
    {}

    void TextureImage::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_texture_unit_compression(helper.getBool(L"Compression", true) ? LITE3D_TRUE : LITE3D_FALSE);

        uint32_t type = textureType(helper.getUpperString(L"TextureType", "2D"));
        uint8_t srgb = helper.getBool(L"sRGB", false) ? LITE3D_TRUE : LITE3D_FALSE;
        uint8_t filtering = textureFiltering(helper.getUpperString(L"Filtering", "NONE"));
        uint8_t wrapping = textureWrap(helper.getUpperString(L"Wrapping", "CLAMPTOEDGE"));

        auto loadImage = [this, type, srgb, filtering, wrapping](const ConfigurationReader &helper)
        {
            lite3d_texture_technique_reset_filters();
            for(const ConfigurationReader &filterConfig : helper.getObjects(L"ProcessingFilters"))
            {
                lite3d_image_filter filter;
                filter.filterID = textureFilterType(filterConfig.getUpperString(L"Type"));
                filter.param1 = (float)filterConfig.getDouble(L"Param1");
                filter.param2 = (float)filterConfig.getDouble(L"Param2");
                lite3d_texture_technique_add_image_filter(&filter);
            }

            if(!lite3d_texture_unit_from_resource(&mTexture, 
                getMain().getResourceManager()->loadFileToMemory(helper.getString(L"Image")),
                textureImageFormat(helper.getUpperString(L"ImageFormat", "ANY")),
                type, 
                srgb,
                filtering, 
                wrapping, 
                helper.getInt(L"CubeFace")))
                LITE3D_THROW(getName() << ": failed to load texture");
        };

        auto cubeFaces = helper.getObjects(L"Image");
        /* load texture from image */
        if (helper.getString(L"Image").size() > 0)
            loadImage(helper);
        /* load cubemap texture */
        else if (cubeFaces.size() > 0)
            std::for_each(cubeFaces.begin(), cubeFaces.end(), loadImage);
        else
        {
            int32_t width = helper.getInt(L"Width", 0), 
                height = helper.getInt(L"Height", 0),
                depth = helper.getInt(L"Depth", 1);
            /* use screen size if not specified */
            if(width == 0 && height == 0)
            {
                int32_t scale = helper.getInt(L"Scale", 1);
                width = getMain().window()->width() / scale;
                height = getMain().window()->height() / scale;
            }

            int32_t samples;
            if ((samples = helper.getInt(L"Samples", 1)) > 1)
            {
                type = type == LITE3D_TEXTURE_2D ? LITE3D_TEXTURE_2D_MULTISAMPLE :
                    (type == LITE3D_TEXTURE_3D ? LITE3D_TEXTURE_3D_MULTISAMPLE : type);
            }

            if (!lite3d_texture_unit_allocate(&mTexture, 
                type, 
                filtering, 
                wrapping, 
                textureFormat(helper.getUpperString(L"TextureFormat", "RGB")),
                textureInternalFormat(helper.getString(L"InternalFormat")),
                width, 
                height, 
                depth, 
                samples))
                LITE3D_THROW(getName() << " failed to allocate new texture");
            
            if(helper.has(L"BlankColor"))
                setBlankColor(helper.getVec4(L"BlankColor"));
        }

        mTexture.userdata = this;
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

                mLayersBackup.emplace_back(pixels);
            }
        }

        Texture::unloadImpl();
    }

    void TextureImage::getPixels(int8_t level, PixelsData &pixels) const
    {
        size_t size;
        if(!lite3d_texture_unit_get_level_size(&mTexture, level, 0, &size))
            LITE3D_THROW("Could`n get size of level " << level << " for texture " << getName());

        pixels.resize(size);
        if(!lite3d_texture_unit_get_pixels(&mTexture, level, 0, &pixels[0]))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::getPixels(int8_t level, void *pixels) const
    {
        if(!lite3d_texture_unit_get_pixels(&mTexture, level, 0, pixels))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::setPixels(int8_t level, const PixelsData &pixels)
    {
        setPixels(level, &pixels[0]);
    }

    void TextureImage::setPixels(int8_t level, const void *pixels)
    {
        if(!lite3d_texture_unit_set_pixels(&mTexture, 0, 0, 0,
            lite3d_texture_unit_get_level_width(&mTexture, level, 0), 
            lite3d_texture_unit_get_level_height(&mTexture, level, 0), 
            lite3d_texture_unit_get_level_depth(&mTexture, level, 0), 
            level, 0, pixels))
            LITE3D_THROW("Could`n set level " << level << " for texture ");

        mModifyed = true;
    }

    void TextureImage::getCompressedPixels(int8_t level, PixelsData &pixels)
    {
        size_t size;
        if(!lite3d_texture_unit_get_compressed_level_size(&mTexture, level, 0, &size))
            LITE3D_THROW("Could`n get size of level " << level << " for texture " << getName());

        pixels.resize(size);
        if(!lite3d_texture_unit_get_compressed_pixels(&mTexture, level, 0, &pixels[0]))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::getCompressedPixels(int8_t level, void *pixels)
    {
        if(!lite3d_texture_unit_get_compressed_pixels(&mTexture, level, 0, pixels))
            LITE3D_THROW("Could`n get level " << level << " for texture ");
    }

    void TextureImage::setCompressedPixels(int8_t level, const PixelsData &pixels)
    {
        setCompressedPixels(level, &pixels[0], pixels.size());
    }

    void TextureImage::setCompressedPixels(int8_t level, const void *pixels, size_t size)
    {
        if(!lite3d_texture_unit_set_compressed_pixels(&mTexture, 0, 0, 0,
            lite3d_texture_unit_get_level_width(&mTexture, level, 0), 
            lite3d_texture_unit_get_level_height(&mTexture, level, 0), 
            lite3d_texture_unit_get_level_depth(&mTexture, level, 0), 
            level, 0, size, pixels))
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
        if(!lite3d_texture_unit_get_level_size(&mTexture, level, 0, &res))
            LITE3D_THROW("Could`n dump level " << level << " size for texture " << getName());

        return res;
    }

    size_t TextureImage::getCompressedLayerSize(int8_t level)
    {
        size_t res;
        if(!lite3d_texture_unit_get_compressed_level_size(&mTexture, level, 0, &res))
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

