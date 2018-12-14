/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <lite3d/lite3d_scene.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_scene_node.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneObject : public Manageable, public Noncopiable
    {
    public:

        typedef std::shared_ptr<SceneObject> Ptr;
        typedef stl<String, SceneNode::Ptr>::map Nodes;

        SceneObject(const String &name, 
            SceneObject *parent, Main *main);
        ~SceneObject();

        inline const Nodes &getNodes() const
        { return mNodes; }
        inline SceneNode *getRoot()
        { return mObjectRoot.get(); }
        inline const SceneNode *getRoot() const
        { return mObjectRoot.get(); }
        inline const String &getName() const 
        { return mName; }

        SceneNode *getNode(const String &name);

        void addToScene(Scene *scene);
        void removeFromScene(Scene *scene);

        void loadFromTemplate(const String &templatePath);

        inline bool isEnabled() const
        { return mEnabled; }

        void disable();
        void enable();
        
    protected:
        
        virtual SceneNode::Ptr createNode(const ConfigurationReader &nodeconf, SceneNode *base);

    private:

        void setupNodes(const stl<ConfigurationReader>::vector &nodesRange, SceneNode *base);

    private:

        String mName;
        Nodes mNodes;
        SceneNode::Ptr mObjectRoot;
        SceneObject *mParent;
        Main *mMain;
        Scene *mScene;
        bool mEnabled;
        std::unique_ptr<ConfigurationReader> mConfiguration;
    };
}

