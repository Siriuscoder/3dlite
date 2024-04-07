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

#include "lite3dpp_physics_common.h"

namespace lite3dpp {
namespace lite3dpp_phisics {

    class PhysicsCollisionShapeSceneNode : public SceneNode
    {
    public:

        using Ptr = std::shared_ptr<PhysicsCollisionShapeSceneNode>;
        enum CollisionShapeType
        {
            Unknown,
            Box,
            Sphere,
            StaticPlane,
            Cylinder,
            Capsule,
            Cone,
            ConvexHull,
            StaticTriangleMesh,
            GimpactTriangleMesh
        };

        PhysicsCollisionShapeSceneNode(const ConfigurationReader &conf, SceneNode *parent, Scene *scene, Main *main);

        inline btCollisionShape *getCollisionShape() { return mCollisionShape.get(); }
        inline btScalar getMass() const { return mShapeMass; }

    protected:

        void setupBoxCollisionShape(const ConfigurationReader& conf);
        void setupStaticPlaneCollisionShape(const ConfigurationReader& conf);
        void setupSphereCollisionShape(const ConfigurationReader& conf);

        std::unique_ptr<btCollisionShape> mCollisionShape;
        btScalar mShapeMass = 0.0f;
        CollisionShapeType mCollisionShapeType = Unknown;
    };

}}
