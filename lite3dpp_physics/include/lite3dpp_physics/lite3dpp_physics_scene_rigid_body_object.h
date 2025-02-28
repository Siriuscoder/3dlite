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

#include <lite3dpp_physics/lite3dpp_physics_scene_object.h>
#include <lite3dpp_physics/lite3dpp_physics_scene_node.h>
#include <lite3dpp_physics/lite3dpp_physics_motion_state.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    class PhysicsRigidBodySceneObject : public PhysicsSceneObject
    {
    public:

        using CollisionNodes = stl<String, PhysicsCollisionShapeSceneNode::Ptr>::unordered_map;

        PhysicsRigidBodySceneObject(const String &name, Scene *scene, Main *main, SceneObjectBase *parent,
            const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale);
        ~PhysicsRigidBodySceneObject();

        void loadFromTemplate(const ConfigurationReader& conf) override;

        virtual SceneNode* addCollisiuonShapeNode(const ConfigurationReader &nodeconf, SceneNodeBase *parent);

        void applyCentralImpulse(const kmVec3 &impulse) override;
        void applyImpulse(const kmVec3 &impulse, const kmVec3 &relativeOffset) override;
        void setLinearVelocity(const kmVec3 &velocity) override;
        virtual kmVec3 getLinearVelocity() const override;

    protected:
        
        SceneNode* createNode(const ConfigurationReader &conf, SceneNodeBase *parent) override;
        btTransform calcRelativeTransform(const SceneNodeBase *node);
        /* Используется если нужно совместить начало координат локальной системы обьекта с центром масс в 
           случае если задана опция пересчета центра масс */
        void changeOrigin(const btVector3 &origin);
        virtual void fillRigidBodyInfo(btRigidBody::btRigidBodyConstructionInfo &info, const ConfigurationReader& conf);

    protected:

        std::unique_ptr<btRigidBody> mBody;
        std::unique_ptr<btCompoundShape> mCompoundCollisionShape;
        btDiscreteDynamicsWorld *mWorld;
        CollisionNodes mCollisionNodes;
        PhysicsObjectMotionState mMotionState;
    };

}}
