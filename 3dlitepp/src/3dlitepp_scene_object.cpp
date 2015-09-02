/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <algorithm>

#include <SDL_log.h>

#include <3dlitepp/3dlitepp_scene.h>
#include <3dlitepp/3dlitepp_scene_object.h>

namespace lite3dpp
{
    SceneObject::SceneObject(const lite3dpp_string &name, 
        SceneObject *parent, Main *main) : 
        mName(name),
        mObjectRoot(NULL),
        mParent(parent),
        mMain(main),
        mScene(NULL),
        mEnabled(true)
    {}

    SceneObject::~SceneObject()
    {
        std::for_each(mNodes.begin(), mNodes.end(), [] (Nodes::value_type &node)
        {
            delete node.second;
        });

        mNodes.clear();
    }

    void SceneObject::loadFromTemplate(const JsonHelper &helper)
    {
        JsonHelper rootNodeHelper = helper.getObject(L"Root");
        if(rootNodeHelper.isEmpty())
            return;

        mObjectRoot = new SceneNode(rootNodeHelper, mParent ? mParent->getRoot() : NULL, mMain);
        setupNodes(rootNodeHelper.getObjects(L"Nodes"), mObjectRoot);
    }

    void SceneObject::setupNodes(const stl<JsonHelper>::vector &nodesRange, SceneNode *base)
    {
        for(const JsonHelper &nodeHelper : nodesRange)
        {
            if(nodeHelper.isEmpty())
                continue;

            SceneNode *sceneNode = new SceneNode(nodeHelper, base, mMain);
            /* create and initialize new node then store it */
            mNodes.insert(std::make_pair(nodeHelper.getString(L"Name"),
                sceneNode));

            stl<JsonHelper>::vector nodesSubRange = nodeHelper.getObjects(L"Nodes");
            if(nodesSubRange.size() > 0)
                setupNodes(nodesSubRange, sceneNode);
        }
    }

    SceneNode *SceneObject::getNode(const lite3dpp_string &name)
    {
        Nodes::iterator it = mNodes.find(name);
        return it == mNodes.end() ? NULL : it->second; 
    }

    void SceneObject::addToScene(Scene *scene)
    {
        if(mScene)
            throw std::runtime_error(getName() + " already in scene " + scene->getName());

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
}

