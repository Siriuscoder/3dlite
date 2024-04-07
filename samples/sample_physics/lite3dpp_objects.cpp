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

#include "lite3dpp_objects.h"
#include "lite3dpp_base.h"

namespace lite3dpp {
namespace samples {

    BaseBody::BaseBody(PhysicSampleBase &sample, const String &name,
        const String &templ) :
        mBase(sample),
        mObj(NULL),
        mDynamic(false)
    {
        mObj = mBase.getScene()->addObject(name, templ, NULL);
    }

    void BaseBody::setPosition(const kmVec3 &pos)
    {
        btVector3 newPos(pos.x, pos.y, pos.z);
        mBody->getWorldTransform().setOrigin(newPos);
        mObj->setPosition(pos);
    }

    void BaseBody::setRotation(const kmQuaternion &rot)
    {
        btQuaternion newRot(rot.x, rot.y, rot.z, rot.w);
        mBody->getWorldTransform().setRotation(newRot);
        mObj->setRotation(rot);
    }

    bool BaseBody::beginSceneRender(Scene *scene, Camera *camera)
    {
        if (mDynamic)
        {
            // update object position and rotation then render begins on this scene
            btVector3 bulletPos = mBody->getWorldTransform().getOrigin();
            btQuaternion bulletRot = mBody->getWorldTransform().getRotation();

            kmVec3 vpos = { bulletPos.getX(), bulletPos.getY(), bulletPos.getZ() };
            kmQuaternion vrot = { bulletRot.getX(), bulletRot.getY(), bulletRot.getZ(), bulletRot.getW() };
            mObj->setPosition(vpos);
            mObj->setRotation(vrot);
        }

        return true;
    }

    void BaseBody::constructBody(float mass)
    {
        // create physical shape of object
        mShape.reset(createShape());
        SDL_assert(mShape);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        mDynamic = fabs(mass) > std::numeric_limits<float>::epsilon();
        btVector3 localInertia(0, 0, 0);
        if (mDynamic)
            mShape->calculateLocalInertia(mass, localInertia);

        btTransform startTransform;
        startTransform.setIdentity();
        mMotionState.reset(new btDefaultMotionState(startTransform));

        // create rigit body
        btRigidBody::btRigidBodyConstructionInfo cInfo(mass, mMotionState.get(), mShape.get(), localInertia);
        reviewRigidBodyConstructionInfo(cInfo);
        mBody.reset(new btRigidBody(cInfo));

        mShape->setUserPointer(this);
        mBody->setUserPointer(this);
        mBase.getScene()->addObserver(this);

        mBase.getWorld()->addRigidBody(mBody.get());
    }

    BaseBody::~BaseBody()
    {
        mBase.getWorld()->removeRigidBody(mBody.get());
        mBase.getScene()->removeObserver(this);
        mBase.getScene()->removeObject(mObj->getName());
    }

    BoxBody::BoxBody(PhysicSampleBase &sample, const String &name) :
        BaseBody(sample, name, "samples:objects/cube.json")
    {
        constructBody(100.0f);
    }

    btCollisionShape *BoxBody::createShape()
    {
        btVector3 half(40.0f, 40.0f, 40.0f);
        return new btBoxShape(half);
    }

    void BoxBody::reviewRigidBodyConstructionInfo(btRigidBody::btRigidBodyConstructionInfo &info)
    {}

    GroundPlaneBody::GroundPlaneBody(PhysicSampleBase &sample, const String &name) :
        BaseBody(sample, name, "samples:objects/ground.json")
    {
        constructBody(0.0f); // ZeroMass
    }

    btCollisionShape *GroundPlaneBody::createShape()
    {
        btVector3 normal(0.0f, 0.0f, 1.0f);
        return new btStaticPlaneShape(normal, 0.0f);
    }

    void GroundPlaneBody::reviewRigidBodyConstructionInfo(btRigidBody::btRigidBodyConstructionInfo &info)
    {}
}}