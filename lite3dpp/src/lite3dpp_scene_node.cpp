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
#include <lite3dpp/lite3dpp_scene_node.h>

#include <SDL_assert.h>
#include <SDL_log.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    SceneNode::SceneNode(const ConfigurationReader &json, SceneNodeBase *parent, Scene *scene) : 
        SceneNodeBase(&mNode)
    {
        SDL_assert(scene);

        lite3d_scene_node_init(&mNode);
        mNode.userdata = this;
        mNode.renderable = LITE3D_FALSE;

        setName(json.getString(L"Name"));
        if (getName().size() == 0)
            LITE3D_THROW("SceneNode with empty name is not allowed..");

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Parsing node '%s' ...", getName().c_str());
        
        frustumTest(json.getBool(L"FrustumTest", true));
        setPosition(json.getVec3(L"Position"));
        setRotation(json.getQuaternion(L"Rotation"));
        scale(json.getVec3(L"Scale", KM_VEC3_ONE));

        /* attach node to scene */
        if (!lite3d_scene_add_node(scene->getPtr(), getPtr(), parent ? parent->getPtr() : nullptr))
        {
            LITE3D_THROW("Attaching node '" << getName() << "' to scene '" << scene->getName() << "' failed..");
        }
    }

    void SceneNode::detachNode()
    {
        lite3d_scene_remove_node(getScene()->getPtr(), &mNode);
    }
}
