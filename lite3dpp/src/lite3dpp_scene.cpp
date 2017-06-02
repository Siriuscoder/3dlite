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
        mLightingParamsBuffer(NULL),
        mLightingIndexBuffer(NULL)
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
        mScene.beforeUpdateNodes = beforeUpdateNodes;
        
        try
        {
            /* default name of lighting buffer is scene name + "LightingBufferObject" */
            mLightingParamsBuffer = mMain->getResourceManager()->
                queryResourceFromJson<TextureBuffer>(getName() + "_lightingBufferObject",
                "{\"BufferFormat\": \"RGBA32F\", \"Dynamic\": true}");
            mLightingIndexBuffer = mMain->getResourceManager()->
                queryResourceFromJson<TextureBuffer>(getName() + "_lightingIndexBuffer",
                "{\"BufferFormat\": \"R16UI\", \"Dynamic\": true}");
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: %s, default lighting will be disabled", LITE3D_CURRENT_FUNCTION, ex.what());
        }

        setupObjects(helper.getObjects(L"Objects"), NULL);
        setupCameras(helper.getObjects(L"Cameras"));
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

        SceneObject::Ptr sceneObject = createObject(name, parent);
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
    
    LightSceneNode *Scene::addLightNode(LightSceneNode *light)
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
    }
    
    LightSceneNode *Scene::getLightNode(const String &name) const
    {
        Lights::const_iterator it;
        if((it = mLights.find(name)) != mLights.end())
            return it->second;

        LITE3D_THROW(name << " object not found");
    }
    
    void Scene::rebuildLightingBuffer()
    {
        if (!mLightingParamsBuffer || !mLightingIndexBuffer || mLights.size() == 0)
            return;

        uint32_t i = 0;

        if (mLightingParamsBuffer->textureBufferSize() < (mLights.size() * sizeof(lite3d_light_params)))
            mLightingParamsBuffer->extendTextureBuffer(sizeof(lite3d_light_params) * mLights.size() / 
                mLightingParamsBuffer->getTexelSize());

        mLightsWorld.resize(mLights.size());

        for (auto &light : mLights)
        {
            mLightingParamsBuffer->setElement<lite3d_light_params>(i, &light.second->getLight()->getPtr()->params);
            mLightsWorld[i] = light.second->getLight()->getPtr()->params;
            light.second->getLight()->index(i++);
        }
    }
    
    void Scene::validateLightingBuffer(const Camera &camera)
    {
        if (!mLightingParamsBuffer || !mLightingIndexBuffer || mLights.size() == 0)
            return;

        // check index buffer size, extend it if needed
        if (mLightingIndexBuffer->textureBufferTexelsCount() < mLights.size()+1)
            mLightingIndexBuffer->extendTextureBuffer(mLights.size()-mLightingIndexBuffer->textureBufferTexelsCount()+1);
        mLightsIndexes.resize(mLights.size()+1);

        uint16_t indexCount = 1;
        bool anyValidated = false;
        for (auto &light : mLights)
        {
            if (light.second->needRecalcToWorld())
            {
                lite3d_light_params wpar = light.second->lightSourceToWorld();
                mLightingParamsBuffer->setElement<lite3d_light_params>(light.second->getLight()->index(), 
                    &wpar);
                light.second->getLight()->validate();
                mLightsWorld[light.second->getLight()->index()] = wpar;
                anyValidated = true;
            }

            if (camera.inFrustum(mLightsWorld[light.second->getLight()->index()]))
                mLightsIndexes[indexCount++] = light.second->getLight()->index();
        }
        
        // the first index contain indexes count, max 16k
        mLightsIndexes[0] = mLightsIndexes.size()-1;
        // upload indexes
        mLightingIndexBuffer->setData(&mLightsIndexes[0], 0, mLightsIndexes.size() * sizeof(LightsIndexesStore::value_type));

        if (anyValidated)
            Material::setIntGlobalParameter(getName() + "_numLights", mLights.size());
    }
    
    SceneObject::Ptr Scene::createObject(const String &name, SceneObject *parent)
    {
        return std::shared_ptr<SceneObject>(new SceneObject(name, parent, mMain));
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
                
                uint32_t renderFlags = 0;
                if (renderTargetJson.getBool(L"RenderFirstStage", true))
                    renderFlags |= LITE3D_RENDER_STAGE_FIRST;
                if (renderTargetJson.getBool(L"RenderSecondStage", true))
                    renderFlags |= LITE3D_RENDER_STAGE_SECOND;
                if (renderTargetJson.getBool(L"CleanColorBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_COLOR_BUF;
                if (renderTargetJson.getBool(L"CleanDepthBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_DEPTH_BUF;
                if (renderTargetJson.getBool(L"CleanStencilBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_STENCIL_BUF;
                if (renderTargetJson.getBool(L"DepthTest", true))
                    renderFlags |= LITE3D_RENDER_DEPTH_TEST;
                if (renderTargetJson.getBool(L"ColorOutput", true))
                    renderFlags |= LITE3D_RENDER_COLOR_OUTPUT;
                if (renderTargetJson.getBool(L"DepthOutput", true))
                    renderFlags |= LITE3D_RENDER_DEPTH_OUTPUT;
                if (renderTargetJson.getBool(L"StencilOutput", false))
                    renderFlags |= LITE3D_RENDER_STENCIL_OUTPUT;
        
                renderTarget->addCamera(camera, renderTargetJson.getInt(L"TexturePass"),
                    renderTargetJson.getInt(L"Priority"), renderFlags);
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

    void Scene::instancingMode(bool flag)
    {
        mScene.instancingRender = flag ? 1 : 0;
    }

    int Scene::beginDrawBatch(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, struct lite3d_material *material)
    {
        SDL_assert(scene->userdata);
        SDL_assert(material->userdata);
        SDL_assert(node->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_CHECK_4(reinterpret_cast<Scene *>(scene->userdata), beginDrawBatch, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<SceneNode *>(node->userdata),
                meshChunk,
                reinterpret_cast<Material *>(material->userdata));
            LITE3D_EXT_OBSERVER_RETURN;
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }

        return LITE3D_FALSE;
    }

    void Scene::nodeInFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol, 
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
                boundingVol,
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::nodeOutOfFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
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
                boundingVol,
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::beforeUpdateNodes(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beforeUpdateNodes, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    int Scene::beginSceneRender(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            Camera *cameraObj = reinterpret_cast<Camera *>(camera->userdata);
            reinterpret_cast<Scene *>(scene->userdata)->validateLightingBuffer(*cameraObj);
            
            LITE3D_EXT_OBSERVER_NOTIFY_CHECK_2(reinterpret_cast<Scene *>(scene->userdata), beginSceneRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                cameraObj);
            LITE3D_EXT_OBSERVER_RETURN;
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }

        return LITE3D_FALSE;
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

