/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#pragma once

#include <lite3d/lite3d_texture_unit.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_json_helper.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Texture : public JsonResource, public NoncopiableResource
    {
    public:

        typedef stl<unsigned char>::vector PixelsData;
        typedef stl<PixelsData>::vector LayersData;

        Texture(const String &name, 
            const String &path, Main *main);

        ~Texture();

        inline lite3d_texture_unit *getPtr()
        { return &mTexture; }

        inline int8_t getLevelsNum()
        { return mTexture.generatedMipmaps; }

        void getPixels(int8_t level,PixelsData &pixels);
        void setPixels(int8_t level, const PixelsData &pixels);

        void getCompressedPixels(int8_t level, PixelsData &pixels);
        void setCompressedPixels(int8_t level, const PixelsData &pixels);

        void generateMipmaps();

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override;
        virtual void reloadFromJsonImpl(const JsonHelper &helper) override;
        virtual void unloadImpl() override;

    private:

        lite3d_texture_unit mTexture;
        bool mModifyed;
        LayersData mLayersData;
    };
}

