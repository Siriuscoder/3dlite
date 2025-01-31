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
#include <lite3dpp/lite3dpp_material_multi_render.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    MultiRenderMaterial::MultiRenderMaterial(const String &name, 
        const String &path, Main *main) : 
        Material(name, path, main)
    {}

    void MultiRenderMaterial::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        Material::loadFromConfigImpl(helper);

        if (!getMain().getResourceManager()->resourceExists(MultiRenderMaterialDataBufferName.data()))
        {
            mMaterialDataBuffer = getMain().getResourceManager()->
                queryResourceFromJson<SSBO>(MultiRenderMaterialDataBufferName.data(), "{\"Dynamic\": true}");
        }
        else
        {
            mMaterialDataBuffer = getMain().getResourceManager()->
                queryResource<SSBO>(MultiRenderMaterialDataBufferName.data());
        }
        
        for (auto &passPair : mPasses)
        {
            /* Буфер с инфо по каждой draw команде (матрицы, индексы и тд) */
            /* только создаем слот, сам буфер управляется и устанавливается сценой */
            getParameter(LITE3D_MULTI_RENDER_CHUNK_INVOCATION_BUFFER, LITE3D_SHADER_PARAMETER_SSBO, passPair.first,
                false, true);
            /* Буфер индексов draw команд */
            getParameter(LITE3D_MULTI_RENDER_CHUNK_INVOCATION_INDEX_BUFFER, LITE3D_SHADER_PARAMETER_UBO, passPair.first,
                false, true);

            setSSBOParameter(passPair.first, "MultiRenderMaterialDataBuffer", *mMaterialDataBuffer);
        }
    }
}

