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
#include "lite3dpp_physics_scene.h"

#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

PhysicsScene::PhysicsScene(const String &name, const String &path, Main *main) : 
    Scene(name, path, main)
{
    getMain().addObserver(this);
}

PhysicsScene::~PhysicsScene()
{
    getMain().removeObserver(this);
}

void PhysicsScene::loadFromConfigImpl(const ConfigurationReader &helper)
{
    auto physicsConfig = helper.getObject(L"Physics");

    createCollisionConfiguration(physicsConfig);
    createCollisionSolver(physicsConfig);
    mBroadphase = std::make_unique<btDbvtBroadphase>();
    mWorld = std::make_unique<btDiscreteDynamicsWorld>(mCollisionDispatcher.get(), mBroadphase.get(), 
        mConstraintSolver.get(), mCollisionConfig.get());

    mWorld->setGravity(BulletUtils::convert(physicsConfig.getVec3(L"Gravity")));
    mMaxSubStepCount = physicsConfig.getInt(L"MaxSubStepCount", MaxSubStepCount);
    if (physicsConfig.has(L"ManualFixedStepIntervalMs"))
    {
        mFixedTimeStep = 1000.0f / physicsConfig.getInt(L"ManualFixedStepIntervalMs");
    }
    else
    {
        auto defaultFixedIntervalTimer = getMain().getFixedUpdateTimer();
        if (defaultFixedIntervalTimer)
        {
            mFixedTimeStep = 1000.0f / defaultFixedIntervalTimer->interval;
        }
    }
}

void PhysicsScene::unloadImpl()
{
    Scene::unloadImpl();
    mWorld.reset();
    mBroadphase.reset();
    mConstraintSolver.reset();
    mCollisionDispatcher.reset();
    mCollisionConfig.reset();
}

void PhysicsScene::createCollisionConfiguration(const ConfigurationReader &helper)
{
    mCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
}

void PhysicsScene::createCollisionSolver(const ConfigurationReader &helper)
{
    mCollisionDispatcher = std::make_unique<btCollisionDispatcher>(mCollisionConfig.get());
    mConstraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
}

void PhysicsScene::timerTick(lite3d_timer *timerid)
{
    if (mWorld)
    {
        btScalar deltaSec = timerid->deltaMcs / 1000000.0f;
        mWorld->stepSimulation(deltaSec, mMaxSubStepCount, mFixedTimeStep);
    }
}

SceneObject::Ptr PhysicsScene::createObject(const String &name, SceneObject *parent)
{
    return std::make_shared<PhysicsRigidBodySceneObject>(name, parent, this, &getMain());
}

}}
