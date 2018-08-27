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

    PhysicSampleBase::PhysicSampleBase()
    {
        // init random seed 
        srand(time(NULL));
        mCollisionConfig.reset(new btDefaultCollisionConfiguration());
        mCollisionDispatcher.reset(new btCollisionDispatcher(mCollisionConfig.get()));
        mBroadphase.reset(new btDbvtBroadphase());
        // the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        mConstraintSolver.reset(new btSequentialImpulseConstraintSolver());
        mWorld.reset(new btDiscreteDynamicsWorld(mCollisionDispatcher.get(), mBroadphase.get(), 
            mConstraintSolver.get(), mCollisionConfig.get()));

        mWorld->setGravity(btVector3(0.0f, 0.0f, -15.0f));
    }

    PhysicSampleBase::~PhysicSampleBase()
    {
    }

    void PhysicSampleBase::createScene()
    {
        // load empty scene with floor plane only
        mScene = getMain().getResourceManager()->queryResource<lite3dpp::Scene>("SampleScene",
            "samples:scenes/empty.json");
        setMainCamera(mScene->getCamera("MyCamera"));
        mScene->instancingMode(true);

        mGroundPlane = createGroundPlane("Ground");
    }

    void PhysicSampleBase::shut()
    {
        mGroundPlane.reset();
    }

    void PhysicSampleBase::fixedUpdateTimerTick()
    {
        SDL_assert(mWorld);
        mWorld->stepSimulation(0.1f, 1, 0.1f);
    }

    BaseBody::Ptr PhysicSampleBase::createBox(const String &name)
    {
        return std::make_shared<BoxBody>(*this, name);
    }

    BaseBody::Ptr PhysicSampleBase::createGroundPlane(const String &name)
    {
        return std::make_shared<GroundPlaneBody>(*this, name);
    }
}}