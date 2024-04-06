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
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_scene_light_node.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    LightSceneNode::LightSceneNode(const ConfigurationReader &json, SceneNode *parent, Scene *scene, Main *main) : 
        SceneNode(json, parent, scene, main)
    {
        /* setup object lighting */
        auto lightHelper = json.getObject(L"Light");
        if (!lightHelper.isEmpty())
        {
            mLight = std::make_unique<LightSource>(lightHelper);
        }

        mScene->addLightSource(this);
    }

    void LightSceneNode::translateToWorld()
    {
        SDL_assert(mLight);
        mLight->translateToWorld(getPtr()->worldView);
    }

    bool LightSceneNode::needRecalcToWorld() const
    {
        return mLight->isUpdated() || getPtr()->invalidated;
    }

    void LightSceneNode::detachNode()
    {
        mScene->removeLightSource(this);
        SceneNode::detachNode();
    }
}
