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
#include <lite3dpp_physics/lite3dpp_physics_scene_node.h>

#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_mesh.h>
#include <lite3dpp_physics/lite3dpp_physics_scene.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsCollisionShapeSceneNode::PhysicsCollisionShapeSceneNode(const ConfigurationReader &conf, SceneNodeBase *parent, 
        Scene *scene) : 
        SceneNode(conf, parent, scene)
    {
        ConfigurationReader collisionShapeConf = conf.getObject(L"CollisionShape");
        auto typeString = collisionShapeConf.getUpperString(L"Type");

        if (typeString == "BOX")
        {
            mCollisionShapeType = Box;
            setupBoxCollisionShape(collisionShapeConf);
        }
        else if (typeString == "SPHERE")
        {
            mCollisionShapeType = Sphere;
            setupSphereCollisionShape(collisionShapeConf);
        }
        else if (typeString == "STATICPLANE")
        {
            mCollisionShapeType = StaticPlane;
            setupStaticPlaneCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CYLINDER")
        {
            mCollisionShapeType = Cylinder;
            setupCylinderCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CAPSULE")
        {
            mCollisionShapeType = Capsule;
            setupCapsuleCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CONE")
        {
            mCollisionShapeType = Cone;
            setupConeCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CONVEXHULL")
        {
            mCollisionShapeType = ConvexHull;
            setupConvexHullCollisionShape(collisionShapeConf);
        }
        else if (typeString == "STATICTRIANGLEMESH")
        {
            mCollisionShapeType = StaticTriangleMesh;
            setupStaticTriangleMeshCollisionShape(collisionShapeConf);
        }
        else if (typeString == "GIMPACTTRIANGLEMESH")
        {
            mCollisionShapeType = GimpactTriangleMesh;
            setupGimpactTriangleMeshCollisionShape(collisionShapeConf);
        }
        else
        {
            LITE3D_THROW("CollisionShapeSceneNode: '" << getName() << "' Unknown collision shape type");
        }

        mNodeMass = collisionShapeConf.getDouble(L"Mass");
        getCollisionShape()->setMargin(collisionShapeConf.getDouble(L"Margin", 0.5));
        getCollisionShape()->setUserPointer(this);
    }

    void PhysicsCollisionShapeSceneNode::setupBoxCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 halfExtents = BulletUtils::convert(conf.getVec3(L"HalfExtents", KM_VEC3_ONE));
        mSimpleCollisionShape = std::make_unique<btBoxShape>(halfExtents);
    }

    void PhysicsCollisionShapeSceneNode::setupStaticPlaneCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 normal = BulletUtils::convert(conf.getVec3(L"PlaneNormal", KM_VEC3_POS_Z));
        btScalar planeConstant = conf.getDouble(L"PlaneConstant");
        mSimpleCollisionShape = std::make_unique<btStaticPlaneShape>(normal, planeConstant);
    }

    void PhysicsCollisionShapeSceneNode::setupSphereCollisionShape(const ConfigurationReader& conf)
    {
        btScalar sphereRadius = conf.getDouble(L"Radius");
        mSimpleCollisionShape = std::make_unique<btSphereShape>(sphereRadius);
    }

    void PhysicsCollisionShapeSceneNode::setupConeCollisionShape(const ConfigurationReader& conf)
    {
        btScalar coneRadius = conf.getDouble(L"Radius");
        btScalar coneHeight = conf.getDouble(L"Height");
        mSimpleCollisionShape = std::make_unique<btConeShapeZ>(coneRadius, coneHeight);
    }

    void PhysicsCollisionShapeSceneNode::setupCapsuleCollisionShape(const ConfigurationReader& conf)
    {
        btScalar coneRadius = conf.getDouble(L"Radius");
        btScalar coneHeight = conf.getDouble(L"Height");
        mSimpleCollisionShape = std::make_unique<btCapsuleShapeZ>(coneRadius, coneHeight);
    }

    void PhysicsCollisionShapeSceneNode::setupCylinderCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 halfExtents = BulletUtils::convert(conf.getVec3(L"HalfExtents", KM_VEC3_ONE));
        mSimpleCollisionShape = std::make_unique<btCylinderShapeZ>(halfExtents);
    }

    void PhysicsCollisionShapeSceneNode::setupConvexHullCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(getScene());
        auto &shapeManager = static_cast<PhysicsScene *>(getScene())->getCollisionShapeManager();
        mTriangleCollisionShape = shapeManager.getCollisionShape(PhysicsTriangleCollisionShape::ConvexHull, conf);
    }

    void PhysicsCollisionShapeSceneNode::setupStaticTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(getScene());
        auto &shapeManager = static_cast<PhysicsScene *>(getScene())->getCollisionShapeManager();
        mTriangleCollisionShape = shapeManager.getCollisionShape(PhysicsTriangleCollisionShape::StaticTriangleMesh, conf);
    }

    void PhysicsCollisionShapeSceneNode::setupGimpactTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(getScene());
        auto &shapeManager = static_cast<PhysicsScene *>(getScene())->getCollisionShapeManager();
        mTriangleCollisionShape = shapeManager.getCollisionShape(PhysicsTriangleCollisionShape::GimpactTriangleMesh, conf);
    }

    btCollisionShape *PhysicsCollisionShapeSceneNode::getCollisionShape()
    {
        if (mTriangleCollisionShape)
        {
            return mTriangleCollisionShape;
        }

        SDL_assert(mSimpleCollisionShape);
        return mSimpleCollisionShape.get();
    }
}}
