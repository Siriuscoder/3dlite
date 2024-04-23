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
#include <SDL_log.h>

#include <lite3dpp/lite3dpp_scene_node.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    SceneNode::SceneNode(const ConfigurationReader &json, SceneNode *parent, Scene *scene, Main *main) : 
        mParentNode(parent),
        mScene(scene),
        mMain(main)
    {
        SDL_assert(main);
        SDL_assert(scene);

        lite3d_scene_node_init(&mNode);
        mNode.userdata = this;
        mNode.renderable = LITE3D_FALSE;

        mName = json.getString(L"Name");
        if(mName.size() == 0)
            LITE3D_THROW("SceneNode with empty name is not allowed..");

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Parsing node '%s' ...", mName.c_str());
        
        frustumTest(json.getBool(L"FrustumTest", true));
        setPosition(json.getVec3(L"Position"));
        setRotation(json.getQuaternion(L"Rotation"));
        scale(json.getVec3(L"Scale", KM_VEC3_ONE));

        /* attach node to scene */
        if(!lite3d_scene_add_node(scene->getPtr(), &mNode, mParentNode ? &mParentNode->mNode : nullptr))
            LITE3D_THROW("Attaching node failed..");
    }

    void SceneNode::setPosition(const kmVec3 &position)
    {
        lite3d_scene_node_set_position(&mNode, &position);
    }

    const kmVec3& SceneNode::getPosition() const
    {
        return mNode.position;
    }

    void SceneNode::move(const kmVec3 &position)
    {
        lite3d_scene_node_move(&mNode, &position);
    }

    void SceneNode::moveRelative(const kmVec3 &p)
    {
        lite3d_scene_node_move_relative(&mNode, &p);
    }

    void SceneNode::setRotation(const kmQuaternion &quat)
    {
        lite3d_scene_node_set_rotation(&mNode, &quat);
    }

    const kmQuaternion& SceneNode::getRotation() const
    {
        return mNode.rotation;
    }

    void SceneNode::rotate(const kmQuaternion &quat)
    {
        lite3d_scene_node_rotate(&mNode, &quat);
    }

    void SceneNode::rotateAngle(const kmVec3 &axis, float angle)
    {
        lite3d_scene_node_rotate_angle(&mNode, &axis, angle);
    }

    void SceneNode::scale(const kmVec3 &scale)
    {
        lite3d_scene_node_scale(&mNode, &scale);
    }
    
    void SceneNode::setVisible(bool flag)
    {
        mNode.visible = flag ? LITE3D_TRUE : LITE3D_FALSE;
    }
    
    bool SceneNode::isVisible() const
    {
        return mNode.visible == LITE3D_TRUE;
    }

    void SceneNode::detachNode()
    {
        lite3d_scene_remove_node(mScene->getPtr(), &mNode);
    }

    kmVec3 SceneNode::getWorldPosition() const
    {
        kmVec3 position;
        lite3d_scene_node_get_world_position(&mNode, &position);
        return position;
    }

    kmQuaternion SceneNode::getWorldRotation() const
    {
        kmQuaternion rotation;
        lite3d_scene_node_get_world_rotation(&mNode, &rotation);
        return rotation;
    }

    void SceneNode::rotateY(float angleDelta)
    {
        lite3d_scene_node_rotate_y(&mNode, angleDelta);
    }

    void SceneNode::rotateX(float angleDelta)
    {
        lite3d_scene_node_rotate_x(&mNode, angleDelta);
    }

    void SceneNode::rotateZ(float angleDelta)
    {
        lite3d_scene_node_rotate_z(&mNode, angleDelta);
    }
}

