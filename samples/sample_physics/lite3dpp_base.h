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

        static void initODE();
        static void shutODE();

        PhysicSampleBase();
        virtual ~PhysicSampleBase();

        void createScene() override;
        void fixedUpdateTimerTick() override;
        // this is called by dSpaceCollide when two objects in space are
        // potentially colliding.
        virtual void potentiallyColliding(dGeomID o1, dGeomID o2);

        inline dWorldID getWorld()
        { return mWorld; }

        inline dSpaceID getGlobalColliderSpace()
        { return mGlobalColliderSpace; }

        inline Scene *getScene()
        { return mScene; }

        BaseBody::Ptr createBox(const String &name);

    private:

        static void nearCallback(void *data, dGeomID o1, dGeomID o2);

    protected:

        dWorldID mWorld;
        dSpaceID mGlobalColliderSpace;
        dJointGroupID mContactGroup;
        dGeomID mGroundPlane;
        Scene *mScene;
    };
}}