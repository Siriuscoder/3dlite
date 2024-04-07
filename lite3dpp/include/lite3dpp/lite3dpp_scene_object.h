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

#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_scene_mesh_node.h>
#include <lite3dpp/lite3dpp_scene_light_node.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneObject : public Manageable, public Noncopiable
    {
    public:

        using Ptr = std::shared_ptr<SceneObject>;
        using Nodes = stl<String, SceneNode::Ptr>::unordered_map;
        using LightNodes = stl<String, LightSceneNode::Ptr>::unordered_map;
        using MeshNodes = stl<String, MeshSceneNode::Ptr>::unordered_map;

        SceneObject(const String &name, 
            SceneObject *parent, Scene *scene, Main *main);
        virtual ~SceneObject() = default;

        inline const Nodes &getNodes() const
        { return mNodes; }
        inline const LightNodes& getLightNodes() const 
        { return mLightNodes; }
        inline const MeshNodes& getMeshNodes() const 
        { return mMeshNodes; }
        inline SceneNode *getRoot()
        { return mObjectRoot; }
        inline const SceneNode *getRoot() const
        { return mObjectRoot; }
        inline const String &getName() const 
        { return mName; }
        inline Main *getMain()
        { return mMain; }
        
        const kmVec3& getPosition() const;
        const kmQuaternion& getRotation() const;

        virtual void setPosition(const kmVec3 &position);
        virtual void move(const kmVec3 &position);
        virtual void moveRelative(const kmVec3 &p);
        virtual void setRotation(const kmQuaternion &quat);
        virtual void rotate(const kmQuaternion &quat);
        virtual void rotateAngle(const kmVec3 &axis, float angle);
        virtual void scale(const kmVec3 &scale);

        SceneNode *getNode(const String &name);
        LightSceneNode* getLightNode(const String &name) const;
        MeshSceneNode* getMeshNode(const String &name) const;

        virtual void loadFromTemplate(const String &templateJsonPath);
        virtual void loadFromTemplate(const ConfigurationReader& conf);

        bool isEnabled() const;
        void disable();
        void enable();

        virtual SceneNode* addNode(const ConfigurationReader &nodeconf, SceneNode *parent);
        virtual MeshSceneNode* addMeshNode(const ConfigurationReader &nodeconf, SceneNode *parent);
        virtual LightSceneNode* addLightNode(const ConfigurationReader &nodeconf, SceneNode *parent);
        
        void removeNode(const String &name);
        void removeMeshNode(const String &name);
        void removeLightNode(const String &name);
        void removeAllLightNodes();
        void removeAllMeshNodes();
        virtual void detachAllNodes();

    protected:
        
        virtual SceneNode* createNode(const ConfigurationReader &conf, SceneNode *parent);
        void setupNodes(const stl<ConfigurationReader>::vector &nodesRange, SceneNode *parent);

    private:

        String mName;
        Main *mMain;

    protected:

        Nodes mNodes;
        LightNodes mLightNodes;
        MeshNodes mMeshNodes;
        SceneNode* mObjectRoot;
        SceneObject *mParent;
        Scene *mScene;
    };
}

