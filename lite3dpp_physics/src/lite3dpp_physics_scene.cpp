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
#include <lite3dpp_physics/lite3dpp_physics_scene.h>

#include <SDL_assert.h>
#include <lite3dpp_physics/lite3dpp_physics_scene_rigid_body_object.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsScene::PhysicsScene(const String &name, const String &path, Main *main) : 
        Scene(name, path, main),
        mCollisionShapeManager(*main)
    {
        getMain().addObserver(this);
    }

    PhysicsScene::~PhysicsScene()
    {
        getMain().removeObserver(this);
    }

    void PhysicsScene::loadFromConfigImpl(const ConfigurationReader &conf)
    {
        auto physicsConfig = conf.getObject(L"Physics");

        createCollisionConfiguration(physicsConfig);
        createCollisionSolver(physicsConfig);
        mBroadphase = std::make_unique<btDbvtBroadphase>();
        mWorld = std::make_unique<btDiscreteDynamicsWorld>(mCollisionDispatcher.get(), mBroadphase.get(), 
            mConstraintSolver.get(), mCollisionConfig.get());

        setGravity(physicsConfig.getVec3(L"Gravity"));
        mWorld->setLatencyMotionStateInterpolation(physicsConfig.getBool(L"LatencyMotionStateInterpolation", false));
        mMaxSubStepCount = physicsConfig.getInt(L"MaxSubStepCount", MaxSubStepCount);
        if (physicsConfig.has(L"ManualFixedStepIntervalMs"))
        {
            mFixedTimeStep = physicsConfig.getInt(L"ManualFixedStepIntervalMs") / 1000.0f;
        }
        else
        {
            auto defaultFixedIntervalTimer = getMain().getFixedUpdateTimer();
            if (defaultFixedIntervalTimer)
            {
                mFixedTimeStep = defaultFixedIntervalTimer->interval / 1000.0f;
            }
        }

        btGImpactCollisionAlgorithm::registerAlgorithm(static_cast<btCollisionDispatcher*>(mWorld->getDispatcher()));

        Scene::loadFromConfigImpl(conf);
        mLastSimulationTime = std::chrono::steady_clock::now();
    }

    void PhysicsScene::setGravity(const kmVec3 &gravity)
    {
        SDL_assert(mWorld);
        mWorld->setGravity(BulletUtils::convert(gravity));
    }

    void PhysicsScene::unloadImpl()
    {
        Scene::unloadImpl();
        mCollisionShapeManager.clearCache();
        mWorld.reset();
        mBroadphase.reset();
        mConstraintSolver.reset();
        mCollisionDispatcher.reset();
        mCollisionConfig.reset();
    }

    void PhysicsScene::createCollisionConfiguration(const ConfigurationReader &conf)
    {
        mCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    }

    void PhysicsScene::createCollisionSolver(const ConfigurationReader &conf)
    {
        mCollisionDispatcher = std::make_unique<btCollisionDispatcher>(mCollisionConfig.get());
        mConstraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
    }

    void PhysicsScene::frameEnd()
    {
        /* run simulation at the end of the frame */
        if (mWorld)
        {
            auto timeNow = std::chrono::steady_clock::now();
            btScalar elapsedSec = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - 
                mLastSimulationTime).count() / 1000000.0f;

            mLastSimulationTime = timeNow;
            mWorld->stepSimulation(elapsedSec, mMaxSubStepCount, mFixedTimeStep);
        }
    }

    SceneObject::Ptr PhysicsScene::createObject(const String &name, SceneObjectBase *parent, const kmVec3 &initialPosition, 
        const kmQuaternion &initialRotation, const kmVec3 &initialScale)
    {
        return std::make_shared<PhysicsRigidBodySceneObject>(name, this, &getMain(), parent, initialPosition, initialRotation,
            initialScale);
    }

    PhysicsSceneObject *PhysicsScene::addPhysicsObject(const String &name, const String &templatePath, 
        SceneObject *parent, const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale)
    {
        return static_cast<PhysicsSceneObject *>(addObject(name, templatePath, parent, initialPosition, initialRotation, initialScale));
    }

    PhysicsSceneObject *PhysicsScene::addPhysicsObject(const String &name, const ConfigurationReader &conf, 
        SceneObject *parent, const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale)
    {
        return static_cast<PhysicsSceneObject *>(addObject(name, conf, parent, initialPosition, initialRotation, initialScale));
    }
}}
