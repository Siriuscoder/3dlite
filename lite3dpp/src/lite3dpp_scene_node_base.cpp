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
#include <lite3dpp/lite3dpp_scene_node_base.h>

#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    SceneNodeBase::SceneNodeBase(lite3d_scene_node *node) : 
        mNodePtr(node)
    {}

    Main *SceneNodeBase::getMain()
    {
        if (getScene())
        {
            return &getScene()->getMain();
        }

        return nullptr;
    }

    Scene *SceneNodeBase::getScene()
    {
        SDL_assert(mNodePtr);
        lite3d_scene *scene = static_cast<lite3d_scene *>(mNodePtr->scene);
        if (scene)
        {
            return reinterpret_cast<Scene *>(scene->userdata);
        }

        return nullptr;
    }

    void SceneNodeBase::frustumTest(bool flag)
    {
        SDL_assert(mNodePtr);
        mNodePtr->frustumTest = flag ? LITE3D_TRUE : LITE3D_FALSE; 
    }
    
    const bool SceneNodeBase::frustumTest() const
    {
        SDL_assert(mNodePtr);
        return mNodePtr->frustumTest == LITE3D_TRUE; 
    }

    void SceneNodeBase::setPosition(const kmVec3 &position)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_set_position(mNodePtr, &position);
    }

    const kmVec3& SceneNodeBase::getPosition() const
    {
        SDL_assert(mNodePtr);
        return mNodePtr->position;
    }

    void SceneNodeBase::move(const kmVec3 &position)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_move(mNodePtr, &position);
    }

    void SceneNodeBase::moveRelative(const kmVec3 &p)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_move_relative(mNodePtr, &p);
    }

    void SceneNodeBase::setRotation(const kmQuaternion &quat)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_set_rotation(mNodePtr, &quat);
    }

    const kmQuaternion& SceneNodeBase::getRotation() const
    {
        SDL_assert(mNodePtr);
        return mNodePtr->rotation;
    }

    void SceneNodeBase::rotate(const kmQuaternion &quat)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_rotate(mNodePtr, &quat);
    }

    void SceneNodeBase::rotateAngle(const kmVec3 &axis, float angle)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_rotate_angle(mNodePtr, &axis, angle);
    }

    void SceneNodeBase::scale(const kmVec3 &scale)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_scale(mNodePtr, &scale);
    }
    
    void SceneNodeBase::setVisible(bool flag)
    {
        SDL_assert(mNodePtr);
        mNodePtr->visible = flag ? LITE3D_TRUE : LITE3D_FALSE;
    }
    
    bool SceneNodeBase::isVisible() const
    {
        SDL_assert(mNodePtr);
        return mNodePtr->visible == LITE3D_TRUE;
    }

    void SceneNodeBase::setRenderable(bool flag)
    {
        SDL_assert(mNodePtr);
        mNodePtr->renderable = flag ? LITE3D_TRUE : LITE3D_FALSE;
    }

    bool SceneNodeBase::isRenderable() const
    {
        SDL_assert(mNodePtr);
        return mNodePtr->renderable == LITE3D_TRUE;
    }
    
    kmVec3 SceneNodeBase::getWorldPosition() const
    {
        SDL_assert(mNodePtr);

        kmVec3 position;
        lite3d_scene_node_get_world_position(mNodePtr, &position);
        return position;
    }

    kmQuaternion SceneNodeBase::getWorldRotation() const
    {
        SDL_assert(mNodePtr);

        kmQuaternion rotation;
        lite3d_scene_node_get_world_rotation(mNodePtr, &rotation);
        return rotation;
    }

    void SceneNodeBase::rotateY(float angleDelta)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_rotate_y(mNodePtr, angleDelta);
    }

    void SceneNodeBase::rotateX(float angleDelta)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_rotate_x(mNodePtr, angleDelta);
    }

    void SceneNodeBase::rotateZ(float angleDelta)
    {
        SDL_assert(mNodePtr);
        lite3d_scene_node_rotate_z(mNodePtr, angleDelta);
    }

    SceneNodeBase *SceneNodeBase::getParent()
    {
        SDL_assert(mNodePtr);
        if (mNodePtr->baseNode)
        {
            return reinterpret_cast<SceneNodeBase *>(mNodePtr->baseNode->userdata);
        }

        return nullptr;
    }

    const SceneNodeBase *SceneNodeBase::getParent() const
    {
        SDL_assert(mNodePtr);
        if (mNodePtr->baseNode)
        {
            return reinterpret_cast<SceneNodeBase *>(mNodePtr->baseNode->userdata);
        }

        return nullptr;
    }
}

