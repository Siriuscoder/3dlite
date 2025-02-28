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
#include <lite3dpp_physics/lite3dpp_physics_scene_rigid_body_object.h>

#include <numeric>
#include <SDL_assert.h>
#include <lite3dpp_physics/lite3dpp_physics_scene.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsRigidBodySceneObject::PhysicsRigidBodySceneObject(const String &name, Scene *scene, Main *main, 
        SceneObjectBase *parent, const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale) : 
        PhysicsSceneObject(name, scene, main, parent, initialPosition, initialRotation, initialScale),
        mWorld(static_cast<PhysicsScene *>(scene)->getWorld()),
        mMotionState(this)
    {}

    PhysicsRigidBodySceneObject::~PhysicsRigidBodySceneObject()
    {
        SDL_assert(mWorld);

        if (mBody)
        {
            mWorld->removeRigidBody(mBody.get());
        }

        mCompoundCollisionShape.reset();
    }

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
        mCompoundCollisionShape = std::make_unique<btCompoundShape>(true, static_cast<int>(mCollisionNodes.size()));
        btCollisionShape *collisionShape = mCompoundCollisionShape.get();
        stl<btScalar>::vector shapesMass;
        for (auto &collisionNode : mCollisionNodes)
        {
            shapesMass.push_back(collisionNode.second->getMass());
            /* GimpactTriangleMeshShape does not support btCompoundShape hierarchy */
            if (collisionNode.second->getCollisionShapeType() == PhysicsCollisionShapeSceneNode::GimpactTriangleMesh)
            {
                collisionShape = collisionNode.second->getCollisionShape();
                mCompoundCollisionShape.reset();
                break;
            }
            else
            {
                /* Расчитаем трансформацию всех коллайдеров относительно корня обьекта */
                btTransform relativeTransform = calcRelativeTransform(collisionNode.second.get());
                mCompoundCollisionShape->addChildShape(relativeTransform, collisionNode.second->getCollisionShape());
            }
        }
        
        btVector3 inertiaTensor(0, 0, 0);
        btScalar mass = 0.0f;
        if (mBodyType == BodyDynamic)
        {
            mass = std::accumulate(shapesMass.begin(), shapesMass.end(), 0.0f);
            if (physicsConfig.getBool(L"CalcCenterOfMass", false) && mCompoundCollisionShape)
            {
                btTransform centerOfMassTransform;
                mCompoundCollisionShape->calculatePrincipalAxisTransform(&shapesMass[0],
                    centerOfMassTransform, inertiaTensor);
                changeOrigin(centerOfMassTransform.getOrigin());
            }
            else
            {
                collisionShape->calculateLocalInertia(mass, inertiaTensor);
            }
        }

        btRigidBody::btRigidBodyConstructionInfo cInfo(mass, &mMotionState, collisionShape, inertiaTensor);
        fillRigidBodyInfo(cInfo, physicsConfig);
        mBody = std::make_unique<btRigidBody>(cInfo);

        mBody->setCollisionFlags(btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
        bool defaultDisableDeactivation = false;
        if (mBodyType == BodyKinematic)
        {
            mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT | 
                btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
            defaultDisableDeactivation = true;
        }

        if (physicsConfig.getBool(L"AlwaysActive", defaultDisableDeactivation))
        {
            mBody->setActivationState(DISABLE_DEACTIVATION);
        }

        collisionShape->setUserPointer(this);
        mBody->setUserPointer(this);
        mWorld->addRigidBody(mBody.get());

        if (physicsConfig.has(L"Gravity"))
        {
            mBody->setGravity(BulletUtils::convert(physicsConfig.getVec3(L"Gravity")));
        }

        if (physicsConfig.has(L"AngularFactor"))
        {
            mBody->setAngularFactor(BulletUtils::convert(physicsConfig.getVec3(L"AngularFactor")));
        }
    }

    btTransform PhysicsRigidBodySceneObject::calcRelativeTransform(const SceneNodeBase *node)
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

    SceneNode* PhysicsRigidBodySceneObject::addCollisiuonShapeNode(const ConfigurationReader &conf, SceneNodeBase *parent)
    {
        auto node = std::make_shared<PhysicsCollisionShapeSceneNode>(conf, parent, getScene());
        if (mNodes.count(node->getName()))
            LITE3D_THROW("CollisionShapeSceneNode '" << node->getName() << "' already exists..");

        mCollisionNodes.emplace(node->getName(), node);
        mNodes.emplace(node->getName(), node);
        return node.get();
    }

    SceneNode* PhysicsRigidBodySceneObject::createNode(const ConfigurationReader &conf, SceneNodeBase *parent)
    {
        if (conf.has(L"CollisionShape"))
        {
            return addCollisiuonShapeNode(conf, parent);
        }

        return SceneObject::createNode(conf, parent);
    }

    void PhysicsRigidBodySceneObject::fillRigidBodyInfo(btRigidBody::btRigidBodyConstructionInfo &info, 
        const ConfigurationReader& conf)
    {
        info.m_friction = conf.getDouble(L"Friction", 0.5f);
        info.m_rollingFriction = conf.getDouble(L"RollingFriction", 0.0f);
        info.m_spinningFriction = conf.getDouble(L"SpinningFriction", 0.0f);
        info.m_restitution = conf.getDouble(L"Restitution", 0.0f);
        info.m_linearDamping = conf.getDouble(L"LinearDamping", 0.0f);
        info.m_angularDamping = conf.getDouble(L"AngularDamping", 0.0f);
        info.m_linearSleepingThreshold = conf.getDouble(L"LinearSleepingThreshold", 0.8f);
        info.m_angularSleepingThreshold = conf.getDouble(L"AngularSleepingThreshold", 0.8f);
    }

    void PhysicsRigidBodySceneObject::applyCentralImpulse(const kmVec3 &impulse)
    {
        SDL_assert(mBody);
        mBody->applyCentralImpulse(BulletUtils::convert(impulse));
    }

    void PhysicsRigidBodySceneObject::applyImpulse(const kmVec3 &impulse, const kmVec3 &relativeOffset)
    {
        SDL_assert(mBody);
        mBody->applyImpulse(BulletUtils::convert(impulse), BulletUtils::convert(relativeOffset));
    }

    void PhysicsRigidBodySceneObject::setLinearVelocity(const kmVec3 &velocity)
    {
        SDL_assert(mBody);
        mBody->setLinearVelocity(BulletUtils::convert(velocity));
    }

    kmVec3 PhysicsRigidBodySceneObject::getLinearVelocity() const 
    {
        return BulletUtils::convert(mBody->getLinearVelocity());
    }
}}
