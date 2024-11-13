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

        mChunkInvocationBuffer = getMain().getResourceManager()->
            queryResourceFromJson<SSBO>("MultiRenderChunkInvocationBuffer", "{\"Dynamic\": true}");
        mMaterialDataBuffer = getMain().getResourceManager()->
            queryResourceFromJson<SSBO>("MultiRenderMaterialDataBuffer", "{\"Dynamic\": true}");

        for (auto &passPair : mPasses)
        {
            /* 
               Насильно добавляем нескоторые built-in параметры чтобы не писать их каждый раз в json 
               projectionMatrix viewMatrix projViewMatrix - могут пригодится, остальные матрицы точно не пригодятся в 
               режиме мультирендера так как будут браться из буферов по DrawID
            */
            getParameter("projectionMatrix", 0, passPair.first, true, true);
            getParameter("viewMatrix", 0, passPair.first, true, true);
            getParameter("projViewMatrix", 0, passPair.first, true, true);

            setSSBOParameter(passPair.first, "MultiRenderChunkInvocationBuffer", *mChunkInvocationBuffer);
            setSSBOParameter(passPair.first, "MultiRenderMaterialDataBuffer", *mMaterialDataBuffer);
        }

        mMaterial.chunkInvocationBuffer = mChunkInvocationBuffer->getPtr();
        mMaterial.materialDataBuffer = mMaterialDataBuffer->getPtr();
    }
}

