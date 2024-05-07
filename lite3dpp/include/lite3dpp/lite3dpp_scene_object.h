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

#include <lite3dpp/lite3dpp_scene_object_base.h>
#include <lite3dpp/lite3dpp_scene_mesh_node.h>
#include <lite3dpp/lite3dpp_scene_light_node.h>
#include <lite3dpp/lite3dpp_scene_light_node.h>


namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneObject : public SceneObjectBase
    {
    public:

        using Ptr = std::shared_ptr<SceneObject>;
        using Nodes = stl<String, SceneNode::Ptr>::unordered_map;
        using LightNodes = stl<String, LightSceneNode::Ptr>::unordered_map;
        using MeshNodes = stl<String, MeshSceneNode::Ptr>::unordered_map;

        SceneObject(const String &name, Scene *scene, Main *main, SceneObjectBase *parent, const kmVec3 &initialPosition, 
            const kmQuaternion &initialRotation, const kmVec3 &initialScale);
        virtual ~SceneObject() = default;

        inline const Nodes &getNodes() const
        { return mNodes; }
        inline const LightNodes& getLightNodes() const 
        { return mLightNodes; }
        inline const MeshNodes& getMeshNodes() const 
        { return mMeshNodes; }

        void disable() override;
        void enable() override;
        
        SceneNode *getNode(const String &name);
        LightSceneNode* getLightNode(const String &name) const;
        MeshSceneNode* getMeshNode(const String &name) const;

        void loadFromTemplate(const ConfigurationReader& conf) override;

        virtual SceneNode* addNode(const ConfigurationReader &nodeconf, SceneNodeBase *parent);
        virtual MeshSceneNode* addMeshNode(const ConfigurationReader &nodeconf, SceneNodeBase *parent);
        virtual LightSceneNode* addLightNode(const ConfigurationReader &nodeconf, SceneNodeBase *parent);
        
        void removeNode(const String &name);
        void removeMeshNode(const String &name);
        void removeLightNode(const String &name);
        void removeAllLightNodes();
        void removeAllMeshNodes();

    protected:
        
        virtual SceneNode* createNode(const ConfigurationReader &conf, SceneNodeBase *parent);
        void setupNodes(const stl<ConfigurationReader>::vector &nodesRange, SceneNodeBase *parent);

    protected:

        Nodes mNodes;
        LightNodes mLightNodes;
        MeshNodes mMeshNodes;
    };
}

