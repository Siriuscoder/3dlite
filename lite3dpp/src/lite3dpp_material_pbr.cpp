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
#include <lite3dpp/lite3dpp_material_pbr.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    PBRMaterial::PBRMaterial(const String &name, 
        const String &path, Main *main) : 
        MultiRenderMaterial(name, path, main)
    {}

    void PBRMaterial::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        MultiRenderMaterial::loadFromConfigImpl(helper);
    }

    void PBRMaterial::addTexture(Texture *texture, TextureFlags flags)
    {
        SDL_assert(texture);

        const size_t maxTexturesCount = sizeof(RawPBRMaterial::textures) / sizeof(RawTextureHandle);
        bool overflow = true;
        for (size_t i = 0; i < maxTexturesCount; ++i)
        {
            if ((mMaterialEntity.textures[i].flags & TextureFlags::LOADED) == TextureFlags::LOADED)
                continue;

            overflow = false;
            mMaterialEntity.textures[i].flags = flags | TextureFlags::LOADED;
            mMaterialEntity.textures[i].textureHandle = texture->handle();
        }

        if (overflow)
        {
            LITE3D_THROW(getName() << ": No more avaliable texture slots");
        }
    }

    void PBRMaterial::update()
    {

    }
}

