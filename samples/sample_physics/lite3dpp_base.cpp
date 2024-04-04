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
#include <algorithm>
#include <SDL_assert.h>
#include <SDL_log.h>

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

        mWorld->setGravity(btVector3(0.0f, 0.0f, -9.8f));
        //mWorld->setLatencyMotionStateInterpolation(true);
    }

    PhysicSampleBase::~PhysicSampleBase()
    {
    }

    void PhysicSampleBase::createScene()
    {
        // load empty scene with floor plane only
        mScene = getMain().getResourceManager()->queryResource<lite3dpp::Scene>("SampleScene",
            "samples:scenes/empty.json");
        setMainCamera(getMain().getCamera("MyCamera"));

        mGroundPlane = createGroundPlane("Ground");
    }

    void PhysicSampleBase::shut()
    {
        mGroundPlane.reset();
    }

    void PhysicSampleBase::fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard)
    {
        SDL_assert(mWorld);

        //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "mWorld->stepSimulation(), FPS %d, firedPerRound %d, step %f, maxSubSteps %d",
        //    getMain().getRenderStats()->lastFPS, firedPerRound, step, maxSubSteps);
        float timeStepSec = (deltaMcs / 1000000.0f);
        mWorld->stepSimulation(timeStepSec, 0);
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