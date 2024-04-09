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
#include <numeric>
#include <SDL_assert.h>

#include "lite3dpp_physics_scene_rigid_body_object.h"
#include "lite3dpp_physics_scene.h"

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsRigidBodySceneObject::PhysicsRigidBodySceneObject(const String &name, Scene *scene, SceneObject *parent,
        const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale) : 
        SceneObject(name, scene, parent, initialPosition, initialRotation, initialScale),
        mWorld(static_cast<PhysicsScene *>(scene)->getWorld()),
        mMotionState(this)
    {}

    void PhysicsRigidBodySceneObject::loadFromTemplate(const ConfigurationReader& conf)
    {
        SDL_assert(mWorld);
        SceneObject::loadFromTemplate(conf);

        ConfigurationReader physicsConfig = conf.getObject(L"Root").getObject(L"Physics");
        if (physicsConfig.isEmpty())
            return;
        
        auto typeString = physicsConfig.getUpperString(L"Type", "STATIC");
        mBodyType = (typeString == "STATIC" ? BodyStatic : (typeString == "DYNAMIC" ? BodyDynamic : BodyKinematic));
        
        if (mCollisionNodes.size() == 0)
        {
            LITE3D_THROW("RigidBodySceneObject: '" << getName() << "' at least one collision shape must be provided");
        }

        /* Придание физической формы для симуляуии, форма может быть составная из нескольких примитивов  */
        mCompoundCollisionShape = std::make_unique<btCompoundShape>(true, mCollisionNodes.size());
        stl<btScalar>::vector shapesMass;
        for (auto &collisionNode : mCollisionNodes)
        {
            /* Расчитаем трансформацию всех коллайдеров относительно корня обьекта */
            btTransform relativeTransform = calcRelativeTransform(collisionNode.second.get());
            mCompoundCollisionShape->addChildShape(relativeTransform, collisionNode.second->getCollisionShape());
            shapesMass.push_back(collisionNode.second->getMass());
        }

        btVector3 inertiaTensor(0, 0, 0);
        btScalar mass = 0.0f;
        if (mBodyType == BodyDynamic)
        {
            mass = std::accumulate(shapesMass.begin(), shapesMass.end(), 0.0f);
            if (physicsConfig.getBool(L"CalcCenterOfMass", false))
            {
                btTransform centerOfMassTransform;
                mCompoundCollisionShape->calculatePrincipalAxisTransform(&shapesMass[0],
                    centerOfMassTransform, inertiaTensor);
                changeOrigin(centerOfMassTransform.getOrigin());
            }
            else
            {
                mCompoundCollisionShape->calculateLocalInertia(mass, inertiaTensor);
            }
        }

        btRigidBody::btRigidBodyConstructionInfo cInfo(mass, &mMotionState, mCompoundCollisionShape.get(), inertiaTensor);
        fillRigidBodyInfo(cInfo, physicsConfig);
        mBody = std::make_unique<btRigidBody>(cInfo);

        mCompoundCollisionShape->setUserPointer(this);
        mBody->setUserPointer(this);
        mWorld->addRigidBody(mBody.get());
    }

    btTransform PhysicsRigidBodySceneObject::calcRelativeTransform(const SceneNode *node)
    {
        if (!node)
        {
            btTransform nodeTransform;
            nodeTransform.setIdentity();
            return nodeTransform;
        }

        btTransform nodeTransform(BulletUtils::convert(node->getRotation()), BulletUtils::convert(node->getPosition()));
        if (node->getParent() == getRoot())
        {
            return nodeTransform;
        }

        return calcRelativeTransform(node->getParent()) * nodeTransform;
    }

    void PhysicsRigidBodySceneObject::changeOrigin(const btVector3 &origin)
    {
        btTransform originTransform(BulletUtils::convert(KM_QUATERNION_IDENTITY), origin);
        originTransform = originTransform.inverse();

        // Поменяем origin у всех привязанных формах в обьекте
        for (int i = 0; i < mCompoundCollisionShape->getNumChildShapes(); ++i)
        {
            auto localTransform = mCompoundCollisionShape->getChildTransform(i);
            mCompoundCollisionShape->updateChildTransform(i, localTransform * originTransform);
        }

        for (auto &node : mNodes)
        {
            // Трансформации подлежат только ноды первого уровня, остальные получют трасформацию автоматически
            if (node.second->getParent() == getRoot())
            {
                node.second->move(BulletUtils::convert(originTransform.getOrigin()));
            }
        }
    }

    void PhysicsRigidBodySceneObject::detachAllNodes()
    {
        SDL_assert(mWorld);

        SceneObject::detachAllNodes();

        if (mBody)
        {
            mWorld->removeRigidBody(mBody.get());
        }
    }

    SceneNode* PhysicsRigidBodySceneObject::addCollisiuonShapeNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        auto node = std::make_shared<PhysicsCollisionShapeSceneNode>(conf, parent, mScene, getMain());
        if (mNodes.count(node->getName()))
            LITE3D_THROW("CollisionShapeSceneNode '" << node->getName() << "' already exists..");

        mCollisionNodes.emplace(node->getName(), node);
        mNodes.emplace(node->getName(), node);
        return node.get();
    }

    SceneNode* PhysicsRigidBodySceneObject::createNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        if (conf.has(L"CollisionShape"))
        {
            return addCollisiuonShapeNode(conf, parent);
        }

        return SceneObject::createNode(conf, parent);
    }

    void PhysicsRigidBodySceneObject::fillRigidBodyInfo(btRigidBody::btRigidBodyConstructionInfo &info, 
        const ConfigurationReader& conf)
    {}
}}
