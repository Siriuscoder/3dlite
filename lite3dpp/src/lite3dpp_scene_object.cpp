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
#include <algorithm>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>
#include <lite3dpp/lite3dpp_scene_object.h>

namespace lite3dpp
{
    SceneObject::SceneObject(const String &name, 
        SceneObject *parent, Scene *scene, Main *main) : 
        mName(name),
        mMain(main),
        mParent(parent),
        mScene(scene)
    {}

    void SceneObject::loadFromTemplate(const String &templatePath)
    {
        size_t fileSize = 0;
        const void *fileData = mMain->getResourceManager()->loadFileToMemory(templatePath, &fileSize);
        ConfigurationReader conf(static_cast<const char *>(fileData), fileSize);
        loadFromTemplate(conf);
    }

    void SceneObject::loadFromTemplate(const ConfigurationReader& conf)
    {
        ConfigurationReader rootNodeHelper = conf.getObject(L"Root");
        if(rootNodeHelper.isEmpty())
            return;

        mObjectRoot = createNode(rootNodeHelper, mParent ? mParent->getRoot() : nullptr);
        setupNodes(rootNodeHelper.getObjects(L"Nodes"), mObjectRoot);
    }

    void SceneObject::setupNodes(const stl<ConfigurationReader>::vector &nodesRange, SceneNode *parent)
    {
        for (const ConfigurationReader &nodeHelper : nodesRange)
        {
            if(nodeHelper.isEmpty())
                continue;

            SceneNode* sceneNode = createNode(nodeHelper, parent);
            setupNodes(nodeHelper.getObjects(L"Nodes"), sceneNode);
        }
    }

    SceneNode *SceneObject::getNode(const String &name)
    {
        Nodes::iterator it = mNodes.find(name);
        return it == mNodes.end() ? nullptr : it->second.get(); 
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
    
    SceneNode* SceneObject::createNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        if (conf.has(L"Mesh"))
            return addMeshNode(conf, parent);
        else if (conf.has(L"Light"))
            return addLightNode(conf, parent);
        
        return addNode(conf, parent);
    }

    SceneNode* SceneObject::addNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        auto node = std::make_shared<SceneNode>(conf, parent, mScene, mMain);
        if (mNodes.count(node->getName()))
            LITE3D_THROW("SceneNode '" << node->getName() << "' already exists..");

        mNodes.emplace(node->getName(), node);
        return node.get();
    }

    MeshSceneNode* SceneObject::addMeshNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        auto meshNode = std::make_shared<MeshSceneNode>(conf, parent, mScene, mMain);
        if (mNodes.count(meshNode->getName()))
            LITE3D_THROW("MeshNode '" << meshNode->getName() << "' already exists..");

        mMeshNodes.emplace(meshNode->getName(), meshNode);
        mNodes.emplace(meshNode->getName(), meshNode);
        return meshNode.get();
    }

    LightSceneNode* SceneObject::addLightNode(const ConfigurationReader &conf, SceneNode *parent)
    {
        auto lightNode = std::make_shared<LightSceneNode>(conf, parent, mScene, mMain);
        if (mNodes.count(lightNode->getName()))
            LITE3D_THROW("LightSource '" << lightNode->getName() << " already exists..");

        mLightNodes.emplace(lightNode->getName(), lightNode);
        mNodes.emplace(lightNode->getName(), lightNode);
        return lightNode.get();
    }

    void SceneObject::removeNode(const String &name)
    {
        if (mNodes.count(name) == 0)
            LITE3D_THROW("Node '" << name << "' is not found..");

        mNodes.erase(name);
    }

    void SceneObject::removeMeshNode(const String &name)
    {
        removeNode(name);
        mMeshNodes.erase(name);
    }
    
    void SceneObject::removeLightNode(const String &name)
    {
        removeNode(name);
        mLightNodes.erase(name);
    }
    
    void SceneObject::removeAllLightNodes()
    {
        while (mLightNodes.size() > 0)
        {
            removeLightNode(mLightNodes.begin()->first);
        }
    }

    void SceneObject::removeAllMeshNodes()
    {
        while (mMeshNodes.size() > 0)
        {
            removeMeshNode(mMeshNodes.begin()->first);
        }
    }

    void SceneObject::detachAllNodes()
    {
        for (auto& node : mNodes)
        {
            node.second->detachNode();
        }
    }
    
    LightSceneNode* SceneObject::getLightNode(const String &name) const
    {
        LightNodes::const_iterator it;
        if((it = mLightNodes.find(name)) != mLightNodes.end())
            return it->second.get();

        LITE3D_THROW("LightNode '" << name << "' is not found");
    }

    MeshSceneNode* SceneObject::getMeshNode(const String &name) const
    {
        MeshNodes::const_iterator it;
        if((it = mMeshNodes.find(name)) != mMeshNodes.end())
            return it->second.get();

        LITE3D_THROW("MeshNode '" << name << "' is not found");
    }

    const kmVec3& SceneObject::getPosition() const
    {
        return getRoot()->getPosition();
    }

    const kmQuaternion& SceneObject::getRotation() const
    {
        return getRoot()->getRotation();
    }

    void SceneObject::setPosition(const kmVec3 &position)
    {
        getRoot()->setPosition(position);
    }

    void SceneObject::move(const kmVec3 &position)
    {
        getRoot()->move(position);
    }

    void SceneObject::moveRelative(const kmVec3 &offset)
    {
        getRoot()->moveRelative(offset);
    }

    void SceneObject::setRotation(const kmQuaternion &quat)
    {
        getRoot()->setRotation(quat);
    }

    void SceneObject::rotate(const kmQuaternion &quat)
    {
        getRoot()->rotate(quat);
    }

    void SceneObject::rotateAngle(const kmVec3 &axis, float angle)
    {
        getRoot()->rotateAngle(axis, angle);
    }

    void SceneObject::scale(const kmVec3 &scale)
    {
        getRoot()->scale(scale);
    }
}

