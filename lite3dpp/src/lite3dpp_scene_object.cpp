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
#include <algorithm>

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>
#include <lite3dpp/lite3dpp_scene_object.h>

namespace lite3dpp
{
    SceneObject::SceneObject(const String &name, 
        SceneObject *parent, Main *main) : 
        mName(name),
        mParent(parent),
        mMain(main),
        mScene(NULL),
        mEnabled(true)
    {}

    SceneObject::~SceneObject()
    {}

    void SceneObject::loadFromTemplate(const String &templatePath)
    {
        size_t fileSize = 0;
        const void *fileData = mMain->getResourceManager()->loadFileToMemory(templatePath, &fileSize);
        mConfiguration.reset(new ConfigurationReader(static_cast<const char *>(fileData), fileSize));

        ConfigurationReader rootNodeHelper = mConfiguration->getObject(L"Root");
        if(rootNodeHelper.isEmpty())
            return;

        mObjectRoot = createNode(rootNodeHelper, mParent ? mParent->getRoot() : NULL);
        setupNodes(rootNodeHelper.getObjects(L"Nodes"), mObjectRoot.get());
    }

    void SceneObject::setupNodes(const stl<ConfigurationReader>::vector &nodesRange, SceneNode *base)
    {
        for(const ConfigurationReader &nodeHelper : nodesRange)
        {
            if(nodeHelper.isEmpty())
                continue;

            SceneNode::Ptr sceneNode = createNode(nodeHelper, base);
            /* create and initialize new node then store it */
            mNodes.insert(std::make_pair(sceneNode->getName(),
                sceneNode));

            stl<ConfigurationReader>::vector nodesSubRange = nodeHelper.getObjects(L"Nodes");
            if(nodesSubRange.size() > 0)
                setupNodes(nodesSubRange, sceneNode.get());
        }
    }

    SceneNode *SceneObject::getNode(const String &name)
    {
        Nodes::iterator it = mNodes.find(name);
        return it == mNodes.end() ? NULL : it->second.get(); 
    }

    void SceneObject::addToScene(Scene *scene)
    {
        if (mScene)
            LITE3D_THROW(getName() << " already in scene " << scene->getName());

        mObjectRoot->addToScene(scene);
        std::for_each(mNodes.begin(), mNodes.end(), [scene] (Nodes::value_type &node)
        {
            node.second->addToScene(scene);
        });

        mScene = scene;
    }

    void SceneObject::removeFromScene(Scene *scene)
    {
        std::for_each(mNodes.begin(), mNodes.end(), [scene] (Nodes::value_type &node)
        {
            node.second->removeFromScene(scene);
        });

        mObjectRoot->removeFromScene(scene);

        mScene = NULL;
    }

    void SceneObject::disable()
    {
        for(Nodes::value_type &node : mNodes)
        {
            node.second->getPtr()->enabled = LITE3D_FALSE;
        }

        mObjectRoot->getPtr()->enabled = LITE3D_FALSE;
    }

    void SceneObject::enable()
    {
        for(Nodes::value_type &node : mNodes)
        {
            node.second->getPtr()->enabled = LITE3D_TRUE;
        }

        mObjectRoot->getPtr()->enabled = LITE3D_TRUE;
    }

    bool SceneObject::isEnabled() const
    {
        return mObjectRoot->getPtr()->enabled == LITE3D_TRUE;
    }
    
    SceneNode::Ptr SceneObject::createNode(const ConfigurationReader &nodeconf, SceneNode *base)
    {
        if (nodeconf.has(L"Mesh"))
            return std::shared_ptr<MeshSceneNode>(new MeshSceneNode(nodeconf, base, mMain));
        else if (nodeconf.has(L"Light"))
            return std::shared_ptr<LightSceneNode>(new LightSceneNode(nodeconf, base, mMain));
        
        return std::shared_ptr<SceneNode>(new SceneNode(nodeconf, base, mMain));
    }

    lite3d_bounding_vol SceneObject::calculateBoudingBox()
    {
        lite3d_bounding_vol bv;
        return bv;
    }

    lite3d_bounding_vol SceneObject::calculateBoudingBoxWorld()
    {
        lite3d_bounding_vol bv;
        return bv;
    }
}

