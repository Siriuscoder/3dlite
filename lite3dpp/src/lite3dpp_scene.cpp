/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    Scene::Scene(const String &name, 
        const String &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::SCENE)
    {}

    Scene::~Scene()
    {}
    
    void Scene::loadImpl(const ResourceParameters &params)
    {}

    void Scene::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_scene_init(&mScene);

        setupObjects(helper.getObjects(L"Objects"), NULL);
        setupCameras(helper.getObjects(L"Cameras"));
    }

    void Scene::unloadImpl()
    {
        removeAllCameras();
        removeAllObjects();
        lite3d_scene_purge(&mScene);
    }

    Camera *Scene::addCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
            throw std::runtime_error("Camera \"" + name + "\" already exists..");

        Camera *camera = new Camera(name, mMain);
        lite3d_scene_add_node(&mScene, &camera->getPtr()->cameraNode, NULL);
        mCameras.insert(std::make_pair(name, camera));
        return camera;
    }

    Camera *Scene::getCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
            return it->second;

        throw std::runtime_error(name + " camera not found..");
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

    void Scene::removeCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
        {
            lite3d_scene_remove_node(&mScene, &it->second->getPtr()->cameraNode);
            delete it->second;
            mCameras.erase(it);
        }
    }

    SceneObject *Scene::addObject(const String &name,
        const String &templatePath, SceneObject *parent)
    {
        if(mObjects.find(name) != mObjects.end())
            throw std::runtime_error(name + " make object failed.. already exist");

        size_t fileSize = 0;
        const void *fileData = mMain->getResourceManager()->loadFileToMemory(templatePath, &fileSize);
        JsonHelper json(static_cast<const char *>(fileData), fileSize);

        SceneObject *sceneObject = new SceneObject(name, parent, mMain);
        sceneObject->loadFromTemplate(json);
        sceneObject->addToScene(this);
        mObjects.insert(std::make_pair(name, sceneObject));
        return sceneObject;
    }

    SceneObject *Scene::getObject(const String &name)
    {
        Objects::iterator it;
        if((it = mObjects.find(name)) != mObjects.end())
            return it->second;

        throw std::runtime_error(name + " object not found");
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

    void Scene::removeObject(const String &name)
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
            SceneObject *sceneObj = addObject(objHelper.getString(L"Name"),
                objHelper.getString(L"Object"), base);

            sceneObj->getRoot()->setPosition(objHelper.getVec3(L"Position"));
            sceneObj->getRoot()->setRotation(objHelper.getQuaternion(L"Rotation"));
            sceneObj->getRoot()->scale(objHelper.getVec3(L"Scale", KM_VEC3_ONE));

            setupObjects(objHelper.getObjects(L"Objects"), sceneObj);
        }
    }

    void Scene::setupCameras(const stl<JsonHelper>::vector &cameras)
    {
        for(const JsonHelper &cameraJson : cameras)
        {
            Camera *camera = addCamera(cameraJson.getString(L"Name"));
            RenderTarget *renderTarget = NULL;

            for(const JsonHelper &renderTargetJson : cameraJson.getObjects(L"RenderTargets"))
            {
                String renderTargetName = renderTargetJson.getString(L"Name");
                if(renderTargetName == "Window") 
                    renderTarget = mMain->window();
                else
                {
                    renderTarget = mMain->getResourceManager()->queryResource<TextureRenderTarget>(
                        renderTargetJson.getString(L"Name"),
                        renderTargetJson.getString(L"Path"));
                }

                renderTarget->addCamera(camera, renderTargetJson.getInt(L"TexturePass"),
                    renderTargetJson.getInt(L"Priority"));
            }

            JsonHelper perspectiveOptionsJson = cameraJson.getObject(L"Perspective");
            JsonHelper orthoOptionsJson = cameraJson.getObject(L"Ortho");
            if(!perspectiveOptionsJson.isEmpty())
            {
                camera->setupPerspective(perspectiveOptionsJson.getDouble(L"Znear"),
                    perspectiveOptionsJson.getDouble(L"Zfar"),
                    perspectiveOptionsJson.getDouble(L"Fov"),
                    perspectiveOptionsJson.getDouble(L"Aspect", -1.0) < 0 ? 
                    (float)renderTarget->width() / (float)renderTarget->height() : cameraJson.getDouble(L"Aspect"));
            }
            else if(!orthoOptionsJson.isEmpty())
            {
                camera->setupOrtho(orthoOptionsJson.getDouble(L"Near"),
                    orthoOptionsJson.getDouble(L"Far"),
                    orthoOptionsJson.getDouble(L"Left"),
                    orthoOptionsJson.getDouble(L"Right"),
                    orthoOptionsJson.getDouble(L"Bottom"),
                    orthoOptionsJson.getDouble(L"Top"));
            }

            camera->setPosition(cameraJson.getVec3(L"Position"));
            camera->lookAt(cameraJson.getVec3(L"LookAt"));
        }
    }
}

