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

#include <ctime>
#include "lite3dpp_objects.h"

namespace lite3dpp {
namespace samples {

    class PhysicSampleBase : public Sample
    {
    public:

        PhysicSampleBase();
        virtual ~PhysicSampleBase();

        // load initial scene
        void createScene() override;
        // performed when engine stops working
        void shut() override;
        void fixedUpdateTimerTick() override;

        inline btDiscreteDynamicsWorld *getWorld()
        { return mWorld.get(); }

        inline Scene *getScene()
        { return mScene; }

        BaseBody::Ptr createBox(const String &name);
        BaseBody::Ptr createGroundPlane(const String &name);

    protected:

        std::unique_ptr<btCollisionConfiguration> mCollisionConfig;
        std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
        std::unique_ptr<btBroadphaseInterface> mBroadphase;
        std::unique_ptr<btConstraintSolver> mConstraintSolver;
        std::unique_ptr<btDiscreteDynamicsWorld> mWorld;
        BaseBody::Ptr mGroundPlane;
        Scene *mScene;
    };
}}