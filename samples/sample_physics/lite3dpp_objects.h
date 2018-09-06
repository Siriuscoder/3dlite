/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2017  Sirius (Korolev Nikita)
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

#include <btBulletDynamicsCommon.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

    class PhysicSampleBase;
    class BaseBody : SceneObserver
    {
    public:

        typedef std::shared_ptr<BaseBody> Ptr;
    
        BaseBody(PhysicSampleBase &sample, const String &name,
            const String &templ);
        BaseBody(const BaseBody &other) = delete;

        virtual ~BaseBody();

        void setPosition(const kmVec3 &pos);
        void setRotation(const kmQuaternion &rot);

        bool beginSceneRender(Scene *scene, Camera *camera) override;
        virtual btCollisionShape *createShape() = 0;
        virtual void reviewRigidBodyConstructionInfo(btRigidBody::btRigidBodyConstructionInfo &info) = 0;

        inline bool isDynamic()
        { return mDynamic; }

    protected:

        virtual void constructBody(float mass);

        PhysicSampleBase &mBase;
        std::unique_ptr<btCollisionShape> mShape;
        std::unique_ptr<btMotionState> mMotionState;
        std::unique_ptr<btRigidBody> mBody;
        SceneObject *mObj;
        bool mDynamic;
        btVector3 mLocalInertia;
    };

    class BoxBody : public BaseBody
    {
    public:

        BoxBody(PhysicSampleBase &sample, const String &name);

        virtual btCollisionShape *createShape() override;
        virtual void reviewRigidBodyConstructionInfo(btRigidBody::btRigidBodyConstructionInfo &info) override;
    };

    class GroundPlaneBody: public BaseBody
    {
    public:

        GroundPlaneBody(PhysicSampleBase &sample, const String &name);

        virtual btCollisionShape *createShape() override;
        virtual void reviewRigidBodyConstructionInfo(btRigidBody::btRigidBodyConstructionInfo &info) override;
    };
}}