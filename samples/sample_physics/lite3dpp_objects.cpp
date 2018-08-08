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
#include "lite3dpp_objects.h"
#include "lite3dpp_base.h"

namespace lite3dpp {
namespace samples {

    BaseBody::BaseBody(PhysicSampleBase &sample, const String &name,
        const String &templ) :
        mScene(sample.getScene())
    {
        mObj = mScene->addObject(name, templ, NULL);
        mBody = dBodyCreate(sample.getWorld());
        dBodySetData(mBody, this);
        mScene->addObserver(this);
    }

    BaseBody::BaseBody(const BaseBody &other)
    {}

    void BaseBody::setPosition(const kmVec3 &pos)
    {
        dBodySetPosition(mBody, pos.x, pos.y, pos.z);
        mObj->getRoot()->setPosition(pos);
    }

    void BaseBody::setRotation(const kmQuaternion &rot)
    {
        dBodySetQuaternion(mBody, &rot.x);
        mObj->getRoot()->setRotation(rot);
    }

    bool BaseBody::beginSceneRender(Scene *scene, Camera *camera)
    {
        // update object position and rotation then render begins on this scene
        const kmVec3 *pos = reinterpret_cast<const kmVec3 *>(dBodyGetPosition(mBody));
        const kmQuaternion *rot = reinterpret_cast<const kmQuaternion *>(dBodyGetQuaternion(mBody));

        mObj->getRoot()->setPosition(*pos);
        mObj->getRoot()->setRotation(*rot);
        return true;
    }

    BaseBody::~BaseBody()
    {
        mScene->removeObserver(this);
        dBodyDestroy(mBody);
        mScene->removeObject(mObj->getName());
    }

    BoxBody::BoxBody(PhysicSampleBase &sample, const String &name) :
        BaseBody(sample, name, "samples:objects/cube.json")
    {
        //dBodySetPosition(body[0], 0, 0, STARTZ);
        dMassSetBox(&mMass, 1, 80.0f, 80.0f, 80.0f);
        dMassAdjust(&mMass, 1.85f);
        dBodySetMass(mBody, &mMass);
        mGeom = dCreateBox(sample.getGlobalColliderSpace(), 80.0f, 80.0f, 80.0f);
        dGeomSetData(mGeom, this);
        dGeomSetBody(mGeom, mBody);
    }

    BoxBody::~BoxBody()
    {
        dGeomDestroy(mGeom);
    }
}}