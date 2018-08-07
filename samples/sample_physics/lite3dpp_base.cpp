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
#include <SDL_assert.h>

#include "lite3dpp_base.h"

namespace lite3dpp {
namespace samples {

    void PhysicSampleBase::initODE()
    {
        // init random seed 
        srand(time(NULL));

        //dAllocateODEDataForThread(dAllocateMaskAll);
        dInitODE2(0);
    }

    void PhysicSampleBase::shutODE()
    {
        dCloseODE();
    }

    PhysicSampleBase::PhysicSampleBase()
    {
        mWorld = dWorldCreate();
        mGlobalColliderSpace = dHashSpaceCreate(0);
        mContactGroup = dJointGroupCreate(0);
        dWorldSetGravity(mWorld, 0.0f, 0.0f, -0.5);
        mGroundPlane = dCreatePlane(mGlobalColliderSpace, 0.0f, 0.0f, 1.0f, 0.0f);
    }

    PhysicSampleBase::~PhysicSampleBase()
    {
        dGeomDestroy(mGroundPlane);
        dJointGroupDestroy(mContactGroup);
        dSpaceDestroy(mGlobalColliderSpace);
        dWorldDestroy(mWorld);
    }

    void PhysicSampleBase::createScene()
    {
        // load empty scene with floor plane only
        mScene = getMain().getResourceManager()->queryResource<lite3dpp::Scene>("SampleScene",
            "samples:scenes/ground.json");
        setMainCamera(mScene->getCamera("MyCamera"));
        mScene->instancingMode(true);
    }

    void PhysicSampleBase::fixedUpdateTimerTick()
    {
        // do colliding detection
        dSpaceCollide(mGlobalColliderSpace, this, &nearCallback);
        // do simulation
        dWorldStep(mWorld, 0.85);

        // remove all contact joints
        dJointGroupEmpty(mContactGroup);
    }

    void PhysicSampleBase::potentiallyColliding(dGeomID o1, dGeomID o2)
    {
        const int N = 20;
        dContact contact[N];

//        BaseBody *body1 = static_cast<BaseBody *>(dGeomGetData(o1));
//        BaseBody *body2 = static_cast<BaseBody *>(dGeomGetData(o2));

//        SDL_assert(body1);
//        SDL_assert(body2);

        int contactsCount = dCollide(o1, o2, N, &contact[0].geom, sizeof(dContact));
        if (contactsCount > 0)
        {
            for (int i = 0; i < contactsCount; i++)
            {
                contact[i].surface.mode = dContactSlip1 | dContactSlip2 | 
                    dContactMu2 | dContactBounce | dContactSoftCFM | dContactApprox1;
                contact[i].surface.mu = dInfinity;
                contact[i].surface.mu2 = dInfinity;
                contact[i].surface.slip1 = 0.7f;// body1->surfaceSlip();
                contact[i].surface.slip2 = 0.7f;// body2->surfaceSlip();
                contact[i].surface.bounce = 0.2;
                contact[i].surface.bounce_vel = 0.2;
                contact[i].surface.soft_cfm = 0.01;
                dJointID c = dJointCreateContact(mWorld, mContactGroup, &contact[i]);
                dJointAttach(c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
            }
        }
    }

    void PhysicSampleBase::nearCallback(void *data, dGeomID o1, dGeomID o2)
    {
        static_cast<PhysicSampleBase *>(data)->potentiallyColliding(o1, o2);
    }

    BaseBody::Ptr PhysicSampleBase::createBox(const String &name)
    {
        return std::make_shared<BoxBody>(*this, name);
    }
}}