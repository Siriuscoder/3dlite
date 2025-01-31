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
#pragma once

#include <lite3dpp_physics/lite3dpp_physics_bullet.h>

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

        PhysicsCollisionShapeSceneNode(const ConfigurationReader &conf, SceneNodeBase *parent, Scene *scene);
        ~PhysicsCollisionShapeSceneNode() = default;

        btCollisionShape *getCollisionShape();
        inline btScalar getMass() const { return mNodeMass; }
        inline CollisionShapeType getCollisionShapeType() const 
        { return mCollisionShapeType; }

    protected:

        void setupBoxCollisionShape(const ConfigurationReader& conf);
        void setupStaticPlaneCollisionShape(const ConfigurationReader& conf);
        void setupSphereCollisionShape(const ConfigurationReader& conf);
        void setupConeCollisionShape(const ConfigurationReader& conf);
        void setupCapsuleCollisionShape(const ConfigurationReader& conf);
        void setupCylinderCollisionShape(const ConfigurationReader& conf);
        void setupConvexHullCollisionShape(const ConfigurationReader& conf);
        void setupStaticTriangleMeshCollisionShape(const ConfigurationReader& conf);
        void setupGimpactTriangleMeshCollisionShape(const ConfigurationReader& conf);

    private:
        std::unique_ptr<btCollisionShape> mSimpleCollisionShape;
        btCollisionShape *mTriangleCollisionShape = nullptr;
        btScalar mNodeMass = 0.0f;
        CollisionShapeType mCollisionShapeType = Unknown;
    };
}}
