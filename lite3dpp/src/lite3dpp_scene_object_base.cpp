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
#include <lite3dpp/lite3dpp_scene_object_base.h>

#include <algorithm>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    SceneObjectBase::SceneObjectBase(const String &name, Scene *scene, SceneObjectBase *parent, const kmVec3 &initialPosition, 
        const kmQuaternion &initialRotation, const kmVec3 &initialScale) : 
        mName(name),
        mScene(scene),
        mParent(parent),
        mInitialPosition(initialPosition),
        mInitialRotation(initialRotation),
        mInitialScale(initialScale)
    {}

    Main *SceneObjectBase::getMain()
    { 
        return &mScene->getMain(); 
    }
    
    const kmVec3& SceneObjectBase::getPosition() const
    {
        return getRoot()->getPosition();
    }

    const kmQuaternion& SceneObjectBase::getRotation() const
    {
        return getRoot()->getRotation();
    }

    kmVec3 SceneObjectBase::getWorldPosition() const
    {
        return getRoot()->getWorldPosition();
    }

    kmQuaternion SceneObjectBase::getWorldRotation() const
    {
        return getRoot()->getWorldRotation();
    }

    void SceneObjectBase::setPosition(const kmVec3 &position)
    {
        getRoot()->setPosition(position);
    }

    void SceneObjectBase::move(const kmVec3 &position)
    {
        getRoot()->move(position);
    }

    void SceneObjectBase::moveRelative(const kmVec3 &offset)
    {
        getRoot()->moveRelative(offset);
    }

    void SceneObjectBase::setRotation(const kmQuaternion &quat)
    {
        getRoot()->setRotation(quat);
    }

    void SceneObjectBase::rotate(const kmQuaternion &quat)
    {
        getRoot()->rotate(quat);
    }

    void SceneObjectBase::rotateAngle(const kmVec3 &axis, float angle)
    {
        getRoot()->rotateAngle(axis, angle);
    }

    void SceneObjectBase::scale(const kmVec3 &scale)
    {
        getRoot()->scale(scale);
    }

    void SceneObjectBase::rotateY(float angleDelta)
    {
        getRoot()->rotateY(angleDelta);
    }

    void SceneObjectBase::rotateX(float angleDelta)
    {
        getRoot()->rotateX(angleDelta);
    }

    void SceneObjectBase::rotateZ(float angleDelta)
    {
        getRoot()->rotateZ(angleDelta);
    }
}

