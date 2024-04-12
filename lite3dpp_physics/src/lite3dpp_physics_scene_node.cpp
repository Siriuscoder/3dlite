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

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsCollisionShapeSceneNode::PhysicsCollisionShapeSceneNode(const ConfigurationReader &conf, SceneNode *parent, 
        Scene *scene, Main *main) : 
        SceneNode(conf, parent, scene, main)
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
            mCollisionShapeType = Cylinder;
        else if (typeString == "CAPSULE")
            mCollisionShapeType = Capsule;
        else if (typeString == "CONE")
            mCollisionShapeType = Cone;
        else if (typeString == "CONVEXHULL")
            mCollisionShapeType = ConvexHull;
        else if (typeString == "STATICTRIANGLEMESH")
            mCollisionShapeType = StaticTriangleMesh;
        else if (typeString == "GIMPACTTRIANGLEMESH")
            mCollisionShapeType = GimpactTriangleMesh;
        else
            LITE3D_THROW("CollisionShapeSceneNode: '" << getName() << "' Unknown collision shape type");

        mShapeMass = collisionShapeConf.getDouble(L"Mass");

        if (mCollisionShape)
        {
            mCollisionShape->setUserPointer(this);
        }
    }

    void PhysicsCollisionShapeSceneNode::setupBoxCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 halfExtents = BulletUtils::convert(conf.getVec3(L"HalfExtents", KM_VEC3_ONE));
        mCollisionShape = std::make_unique<btBoxShape>(halfExtents);
    }

    void PhysicsCollisionShapeSceneNode::setupStaticPlaneCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 normal = BulletUtils::convert(conf.getVec3(L"PlaneNormal", KM_VEC3_POS_Z));
        btScalar planeConstant = conf.getDouble(L"PlaneConstant");
        mCollisionShape = std::make_unique<btStaticPlaneShape>(normal, planeConstant);
    }

    void PhysicsCollisionShapeSceneNode::setupSphereCollisionShape(const ConfigurationReader& conf)
    {
        btScalar sphereRadius = conf.getDouble(L"Radius");
        mCollisionShape = std::make_unique<btSphereShape>(sphereRadius);
    }
}}
