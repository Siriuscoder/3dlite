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
#pragma once

#include <lite3dpp/lite3dpp_scene_object.h>
#include <lite3dpp_physics/lite3dpp_physics_common.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    class LITE3DPP_PHYSICS_EXPORT PhysicsSceneObject : public SceneObject
    {
    public:

        enum BodyType {
            BadBodyType,
            BodyStatic = 1,
            BodyDynamic, 
            BodyKinematic
        };

        PhysicsSceneObject(const String &name, Scene *scene, Main *main, SceneObjectBase *parent,
            const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale) : 
            SceneObject(name, scene, main, parent, initialPosition, initialRotation, initialScale)
        {}

        virtual ~PhysicsSceneObject() = default;

        inline BodyType getBodyType() const { return mBodyType; }

        virtual void applyCentralImpulse(const kmVec3 &impulse) = 0;
        virtual void applyImpulse(const kmVec3 &impulse, const kmVec3 &relativeOffset) = 0;
        virtual void setLinearVelocity(const kmVec3 &velocity) = 0;

    protected:

        BodyType mBodyType = BadBodyType;
    };

}}
