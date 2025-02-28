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
#include <lite3dpp/lite3dpp_scene_light_node.h>

#include <SDL_assert.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    LightSceneNode::LightSceneNode(const ConfigurationReader &json, SceneNodeBase *parent, Scene *scene) : 
        SceneNode(json, parent, scene)
    {
        /* setup object lighting */
        auto lightHelper = json.getObject(L"Light");
        if (!lightHelper.isEmpty())
        {
            mLight = std::make_unique<LightSource>(lightHelper);
            getScene()->addLightSource(this);
        }
    }

    void LightSceneNode::translateToWorld()
    {
        if (mLight)
        {
            mLight->translateToWorld(getPtr()->worldMatrix);
        }
    }

    bool LightSceneNode::needRecalcToWorld() const
    {
        if (mLight)
        {
            return mLight->isUpdated() || getPtr()->invalidated;
        }

        return getPtr()->invalidated;
    }

    LightSceneNode::~LightSceneNode()
    {
        if (mLight)
        {
            getScene()->removeLightSource(this);
        }
    }
}
