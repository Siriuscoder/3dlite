/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_log.h>

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_texture.h>

namespace lite3dpp
{
    Texture::Texture(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::TEXTURE)
    {}

    Texture::~Texture()
    {}

    void Texture::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_texture_unit_compression(helper.getBool(L"Compression", true) ? LITE3D_TRUE : LITE3D_FALSE);

        lite3dpp_string textureTypeStr = helper.getString(L"TextureType", "2D");
        lite3dpp_string filteringStr = helper.getString(L"Filtering", "None");
        lite3dpp_string wrappingStr = helper.getString(L"Wrapping", "ClampToEdge");
        uint32_t textureType = textureTypeStr == "1D" ? LITE3D_TEXTURE_1D : 
            (textureTypeStr == "2D" ? LITE3D_TEXTURE_2D : LITE3D_TEXTURE_3D);
        uint8_t quality = filteringStr == "None" ? LITE3D_TEXTURE_QL_LOW : 
            (filteringStr == "Linear" ? LITE3D_TEXTURE_QL_MEDIUM : 
            (filteringStr == "Trilinear" ? LITE3D_TEXTURE_QL_NICEST : 0));
        uint8_t wrapping = wrappingStr == "ClampToEdge" ? LITE3D_TEXTURE_CLAMP_TO_EDGE : 
            (wrappingStr == "Repeat" ? LITE3D_TEXTURE_REPEAT : 0);

        /* load texture from image */
        if(helper.getString(L"Image").size() > 0)
        {
            lite3dpp_string imageFormatStr = helper.getString(L"ImageFormat", "ANY");

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
            for(const JsonHelper &filterConfig : helper.getObjects(L"ProcessingFilters"))
            {
                lite3d_image_filter filter;
                lite3dpp_string filterTypeStr = filterConfig.getString(L"Type");
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
                throw std::runtime_error(getName() + " texture load failed..");
        }
        else
        {
            lite3dpp_string textureFormatStr = helper.getString(L"TextureFormat", "RGB");
            uint16_t textureFormat = textureFormatStr == "ALPHA" ? LITE3D_TEXTURE_FORMAT_ALPHA :
                (textureFormatStr == "RGB" ? LITE3D_TEXTURE_FORMAT_RGB : 
                (textureFormatStr == "RGBA" ? LITE3D_TEXTURE_FORMAT_RGBA : 
                (textureFormatStr == "BRG" ? LITE3D_TEXTURE_FORMAT_BRG : 
                (textureFormatStr == "BRGA" ? LITE3D_TEXTURE_FORMAT_BRGA : 
                (textureFormatStr == "LUMINANCE" ? LITE3D_TEXTURE_FORMAT_LUMINANCE : 
                (textureFormatStr == "LUMINANCE_ALPHA" ? LITE3D_TEXTURE_FORMAT_LUMINANCE_ALPHA :
                (textureFormatStr == "DEPTH" ? LITE3D_TEXTURE_FORMAT_DEPTH : 0)))))));

            if(!lite3d_texture_unit_allocate(&mTexture, textureType, quality, wrapping, textureFormat, 
                helper.getInt(L"Height"), helper.getInt(L"Width"), helper.getInt(L"Depth")))
                throw std::runtime_error(getName() + " texture allocation failed..");
        }

        mTexture.userdata = this;
    }

    void Texture::unloadImpl()
    {
        lite3d_texture_unit_purge(&mTexture);
    }
}

