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

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_scene.h>

namespace lite3dpp
{
    Scene::Scene(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::SCENE)
    {}

    Scene::~Scene()
    {}

    void Scene::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_scene_init(&mScene);

        setupObjects(helper.getObjects(L"Objects"), NULL);
    }

    void Scene::unloadImpl()
    {
        removeAllCameras();
        removeAllObjects();
        lite3d_scene_purge(&mScene);
    }

    Camera *Scene::getCamera(const lite3dpp_string &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
            return it->second;

        Camera *camera = new Camera(name, mMain);
        lite3d_scene_add_node(&mScene, &camera->getPtr()->cameraNode, NULL);
        mCameras.insert(std::make_pair(name, camera));
        return camera;
    }

    void Scene::removeAllCameras()
    {
        std::for_each(mCameras.begin(), mCameras.end(), [this](Cameras::value_type &camera)
        {
            lite3d_scene_remove_node(&mScene, &camera.second->getPtr()->cameraNode);
            delete camera.second;
        });

        mCameras.clear();
    }

    void Scene::removeCamera(const lite3dpp_string &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
        {
            lite3d_scene_remove_node(&mScene, &it->second->getPtr()->cameraNode);
            delete it->second;
            mCameras.erase(it);
        }
    }

    SceneObject *Scene::makeObject(const lite3dpp_string &name,
        const lite3dpp_string &templatePath, SceneObject *parent)
    {
        if(mObjects.find(name) != mObjects.end())
            throw std::runtime_error(name + " make object failed.. already exist");

        size_t fileSize = 0;
        JsonHelper json(static_cast<const char *>(
            mMain->getResourceManager()->loadFileToMemory(templatePath, &fileSize)), fileSize);

        SceneObject *sceneObject = new SceneObject(name, parent, mMain);
        sceneObject->loadFromTemplate(json);
        sceneObject->addToScene(this);
        mObjects.insert(std::make_pair(name, sceneObject));
        return sceneObject;
    }

    SceneObject *Scene::getObject(const lite3dpp_string &name)
    {
        Objects::iterator it;
        if((it = mObjects.find(name)) != mObjects.end())
            return it->second;

        return NULL;
    }

    void Scene::removeAllObjects()
    {
        for(Objects::value_type &object : mObjects)
        {
            object.second->removeFromScene(this);
            delete object.second;
        }

        mObjects.clear();
    }

    void Scene::removeObject(const lite3dpp_string &name)
    {
        Objects::iterator it;
        if((it = mObjects.find(name)) == mObjects.end())
            throw std::runtime_error(name + " remove object failed.. not found");
        it->second->removeFromScene(this);
        delete it->second;

        mObjects.erase(it);
    }

    void Scene::setupObjects(const stl<JsonHelper>::vector &objects, SceneObject *base)
    {
        for(const JsonHelper &objHelper : objects)
        {
            if(objHelper.isEmpty())
                continue;
            SceneObject *sceneObj = makeObject(objHelper.getString(L"Name"),
                objHelper.getString(L"Object"), base);

            if(!objHelper.getObject(L"Position").isEmpty())
                sceneObj->getRoot()->setPosition(objHelper.getVec3(L"Position"));
            if(!objHelper.getObject(L"Rotation").isEmpty())
                sceneObj->getRoot()->setRotation(objHelper.getQuaternion(L"Rotation"));
            if(!objHelper.getObject(L"Scale").isEmpty())
                sceneObj->getRoot()->scale(objHelper.getVec3(L"Scale"));

            setupObjects(objHelper.getObjects(L"Objects"), sceneObj);
        }
    }
}

