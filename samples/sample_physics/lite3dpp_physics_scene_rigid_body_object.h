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

#include "lite3dpp_physics_scene_node.h"
#include "lite3dpp_physics_motion_state.h"

namespace lite3dpp {
namespace lite3dpp_phisics {

    class PhysicsRigidBodySceneObject : public SceneObject
    {
    public:

        using CollisionNodes = stl<String, PhysicsCollisionShapeSceneNode::Ptr>::unordered_map;

        enum BodyType {
            BadBodyType,
            BodyStatic = 1,
            BodyDynamic, 
            BodyKinematic
        };

        PhysicsRigidBodySceneObject(const String &name, SceneObject *parent, Scene *scene, Main *main);

        inline BodyType getBodyType() const { return mBodyType; }

        void loadFromTemplate(const ConfigurationReader& conf) override;
        void detachAllNodes() override;

        virtual SceneNode* addCollisiuonShapeNode(const ConfigurationReader &nodeconf, SceneNode *parent);
        /* Для динамических и статических обьектов желательно устанавливать только начальную позицию, для кинематик 
           обьектов можно смело вызывать так как физика на них не влияет */
        void setPosition(const kmVec3 &position) override;
        void setRotation(const kmQuaternion &quat) override;

    protected:
        
        SceneNode* createNode(const ConfigurationReader &conf, SceneNode *parent) override;
        btTransform calcRelativeTransform(const SceneNode *node);
        virtual void fillRigidBodyInfo(btRigidBody::btRigidBodyConstructionInfo &info, const ConfigurationReader& conf);

    private:

        BodyType mBodyType = BadBodyType;

    protected:

        std::unique_ptr<btRigidBody> mBody;
        std::unique_ptr<btCompoundShape> mCompoundCollisionShape;
        btDiscreteDynamicsWorld *mWorld;
        CollisionNodes mCollisionNodes;
        PhysicsObjectMotionState mMotionState;
    };

}}
