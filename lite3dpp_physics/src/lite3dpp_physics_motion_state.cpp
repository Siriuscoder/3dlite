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
#include <lite3dpp_physics/lite3dpp_physics_motion_state.h>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

PhysicsObjectMotionState::PhysicsObjectMotionState(SceneObject *o) : 
    mSceneObject(o)
{}

void PhysicsObjectMotionState::getWorldTransform(btTransform& worldTrans) const
{
    SDL_assert(mSceneObject);
    worldTrans.setOrigin(BulletUtils::convert(mSceneObject->getPosition()));
    worldTrans.setRotation(BulletUtils::convert(mSceneObject->getRotation()));
}

void PhysicsObjectMotionState::setWorldTransform(const btTransform& worldTrans)
{
    SDL_assert(mSceneObject);
    mSceneObject->setPosition(BulletUtils::convert(worldTrans.getOrigin()));
    mSceneObject->setRotation(BulletUtils::convert(worldTrans.getRotation()));
}

}}
