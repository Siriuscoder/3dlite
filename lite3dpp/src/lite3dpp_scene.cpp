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
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    Scene::Scene(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::SCENE),
        mLightingTextureBuffer(NULL)
    {}

    Scene::~Scene()
    {}

    void Scene::loadFromConfigImpl(const ConfigurationReader &helper)       
    {
        lite3d_scene_init(&mScene);
        mScene.userdata = this;
        mScene.beginDrawBatch = beginDrawBatch;
        mScene.beginFirstStageRender = beginFirstStageRender;
        mScene.beginSceneRender = beginSceneRender;
        mScene.beginSecondStageRender = beginSecondStageRender;
        mScene.endSceneRender = endSceneRender;
        mScene.nodeInFrustum = nodeInFrustum;
        mScene.nodeOutOfFrustum = nodeOutOfFrustum;

        setupObjects(helper.getObjects(L"Objects"), NULL);
        setupCameras(helper.getObjects(L"Cameras"));
        
        try
        {
            /* default name of lighting buffer is scene name + "LightingBufferObject" */
            mLightingTextureBuffer = mMain->getResourceManager()->
                queryResourceFromJson<TextureBuffer>(getName() + "_lightingBufferObject",
                "{\"BufferFormat\": \"RGBA32F\"}");
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: %s, default lighting disabled", LITE3D_CURRENT_FUNCTION, ex.what());
        }
    }

    void Scene::unloadImpl()
    {
        removeAllCameras();
        removeAllObjects();
        removeAllLights();
        lite3d_scene_purge(&mScene);
    }

    Camera *Scene::addCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
            LITE3D_THROW("Camera \"" << name << "\" already exists..");

        std::shared_ptr<Camera> camera = std::make_shared<Camera>(name, mMain);
        lite3d_scene_add_node(&mScene, &camera->getPtr()->cameraNode, NULL);
        mCameras.insert(std::make_pair(name, camera));
        return camera.get();
    }

    Camera *Scene::getCamera(const String &name) const
    {
        Cameras::const_iterator it = mCameras.find(name);
        if(it != mCameras.end())
            return it->second.get();

        LITE3D_THROW("Camera " << name << " not found..");
    }

    void Scene::removeAllCameras()
    {
        std::for_each(mCameras.begin(), mCameras.end(), [this](Cameras::value_type &camera)
        {
            lite3d_scene_remove_node(&mScene, &camera.second->getPtr()->cameraNode);
        });

        mCameras.clear();
    }

    void Scene::removeCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if(it != mCameras.end())
        {
            lite3d_scene_remove_node(&mScene, &it->second->getPtr()->cameraNode);
            mCameras.erase(it);
        }
    }

    SceneObject *Scene::addObject(const String &name,
        const String &templatePath, SceneObject *parent)
    {
        if(mObjects.find(name) != mObjects.end())
            LITE3D_THROW(name << " make object failed.. already exist");

        std::shared_ptr<SceneObject> sceneObject = std::make_shared<SceneObject>(name, parent, mMain);
        sceneObject->loadFromTemplate(templatePath);
        sceneObject->addToScene(this);
        mObjects.insert(std::make_pair(name, sceneObject));
        return sceneObject.get();
    }

    SceneObject *Scene::getObject(const String &name) const
    {
        Objects::const_iterator it;
        if((it = mObjects.find(name)) != mObjects.end())
            return it->second.get();

        LITE3D_THROW(name << " object not found");
    }

    void Scene::removeAllObjects()
    {
        for(Objects::value_type &object : mObjects)
        {
            object.second->removeFromScene(this);
        }

        mObjects.clear();
    }

    void Scene::removeObject(const String &name)
    {
        Objects::iterator it;
        if((it = mObjects.find(name)) == mObjects.end())
            LITE3D_THROW(name << " remove object failed.. not found");
        it->second->removeFromScene(this);

        mObjects.erase(it);
    }
    
    SceneNode *Scene::addLightNode(SceneNode *light)
    {
        Lights::iterator it = mLights.find(light->getName());
        if(it != mLights.end())
            LITE3D_THROW("LightSource \"" << light->getName() << "\" already exists..");
        if(!light->getLight())
            LITE3D_THROW("Node \"" << light->getName() << "\" do not contain light source");

        mLights.insert(std::make_pair(light->getName(), light));
        rebuildLightingBuffer();
        return light;
    }
    
    void Scene::removeLight(const String &name)
    {
        Lights::iterator it = mLights.find(name);
        if(it != mLights.end())
        {
            mLights.erase(it);
            rebuildLightingBuffer();
        }
    }
    
    void Scene::removeAllLights()
    {
        mLights.clear();
        rebuildLightingBuffer();
    }
    
    SceneNode *Scene::getLightNode(const String &name) const
    {
        Lights::const_iterator it;
        if((it = mLights.find(name)) != mLights.end())
            return it->second;

        LITE3D_THROW(name << " object not found");
    }
    
    void Scene::rebuildLightingBuffer()
    {
        uint32_t i = 0;
        for (auto &light : mLights)
        {
            /* extend if needed */
            if (mLightingTextureBuffer->textureBufferSize() < ((i+1) * sizeof(lite3d_light_params)))
            {
                mLightingTextureBuffer->extendTextureBuffer(sizeof(lite3d_light_params) / 
                    mLightingTextureBuffer->getTexelSize());
            }
            
            mLightingTextureBuffer->setElement<lite3d_light_params>(i, &light.second->getLight()->getPtr()->params);
            light.second->getLight()->validate();
            light.second->getLight()->index(i++);
        }
        
        Material::setIntGlobalParameter(getName() + "_numLights", i);
    }
    
    void Scene::validateLightingBuffer()
    {
        for (auto &light : mLights)
        {
            if (light.second->getLight()->isUpdated())
            {
                mLightingTextureBuffer->setElement<lite3d_light_params>(light.second->getLight()->index(), 
                    &light.second->getLight()->getPtr()->params);
                light.second->getLight()->validate();
            }
        }      
    }

    void Scene::setupObjects(const stl<ConfigurationReader>::vector &objects, SceneObject *base)
    {
        for(const ConfigurationReader &objHelper : objects)
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

    void Scene::setupCameras(const stl<ConfigurationReader>::vector &cameras)
    {
        for(const ConfigurationReader &cameraJson : cameras)
        {
            Camera *camera = addCamera(cameraJson.getString(L"Name"));
            RenderTarget *renderTarget = NULL;

            for(const ConfigurationReader &renderTargetJson : cameraJson.getObjects(L"RenderTargets"))
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

            ConfigurationReader perspectiveOptionsJson = cameraJson.getObject(L"Perspective");
            ConfigurationReader orthoOptionsJson = cameraJson.getObject(L"Ortho");
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

            if(cameraJson.has(L"Position"))
                camera->setPosition(cameraJson.getVec3(L"Position"));
            if(cameraJson.has(L"LookAt"))
                camera->lookAt(cameraJson.getVec3(L"LookAt"));
        }
    }

    void Scene::beginDrawBatch(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, struct lite3d_material *material)
    {
        SDL_assert(scene->userdata);
        SDL_assert(material->userdata);
        SDL_assert(node->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_4(reinterpret_cast<Scene *>(scene->userdata), beginDrawBatch, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<SceneNode *>(node->userdata),
                meshChunk,
                reinterpret_cast<Material *>(material->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::nodeInFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bouding_vol *boudingVol, 
            struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(material->userdata);
        SDL_assert(node->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_6(reinterpret_cast<Scene *>(scene->userdata), nodeInFrustum, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<SceneNode *>(node->userdata),
                meshChunk,
                reinterpret_cast<Material *>(material->userdata),
                boudingVol,
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::nodeOutOfFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bouding_vol *boudingVol,
            struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(material->userdata);
        SDL_assert(node->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_6(reinterpret_cast<Scene *>(scene->userdata), nodeOutOfFrustum, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<SceneNode *>(node->userdata),
                meshChunk,
                reinterpret_cast<Material *>(material->userdata),
                boudingVol,
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::beginSceneRender(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            reinterpret_cast<Scene *>(scene->userdata)->validateLightingBuffer();
            
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beginSceneRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::endSceneRender(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), endSceneRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::beginFirstStageRender(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beginFirstStageRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::beginSecondStageRender(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beginSecondStageRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }
}

