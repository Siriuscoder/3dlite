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

#include <chrono>

#include <lite3dpp_physics/lite3dpp_physics_common.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    class LITE3DPP_PHYSICS_EXPORT PhysicsScene : public Scene, public LifecycleObserver
    {
    public:

        static constexpr const int MaxSubStepCount = 10;
        static constexpr const float FixedTimeStep = 1.0 / 60.0;

        PhysicsScene(const String &name, const String &path, Main *main);
        ~PhysicsScene();

        inline btDiscreteDynamicsWorld *getWorld() { return mWorld.get(); } 

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &conf) override;
        virtual void unloadImpl() override;
        virtual void createCollisionConfiguration(const ConfigurationReader &conf);
        virtual void createCollisionSolver(const ConfigurationReader &conf);
        virtual void frameEnd() override;
        virtual SceneObject::Ptr createObject(const String &name, SceneObject *parent, const kmVec3 &initialPosition, 
            const kmQuaternion &initialRotation, const kmVec3 &initialScale) override;

    protected:

        std::unique_ptr<btCollisionConfiguration> mCollisionConfig;
        std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
        std::unique_ptr<btBroadphaseInterface> mBroadphase;
        std::unique_ptr<btConstraintSolver> mConstraintSolver;
        std::unique_ptr<btDiscreteDynamicsWorld> mWorld;
        int mMaxSubStepCount = MaxSubStepCount;
        float mFixedTimeStep = FixedTimeStep;
        std::chrono::steady_clock::time_point mLastSimulationTime;
    };
}}
