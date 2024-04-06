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

namespace lite3dpp
{
    Scene::Scene(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::SCENE),
        mLightingParamsBuffer(nullptr),
        mLightingIndexBuffer(nullptr)
    {
        addObserver(this);
    }

    Scene::~Scene()
    {
        removeObserver(this);
    }

    size_t Scene::usedVideoMemBytes() const
    {
        return 0;
    }

    void Scene::setupCallbacks()
    {
        mScene.userdata = this;
        mScene.beginDrawBatch = beginDrawBatchEntry;
        mScene.beginOpaqueStageRender = beginOpaqueStageRenderEntry;
        mScene.beginSceneRender = beginSceneRenderEntry;
        mScene.beginBlendingStageRender = beginBlendingStageRenderEntry;
        mScene.endSceneRender = endSceneRenderEntry;
        mScene.nodeInFrustum = nodeInFrustumEntry;
        mScene.nodeOutOfFrustum = nodeOutOfFrustumEntry;
        mScene.customVisibilityCheck = customVisibilityCheckEntry;
        mScene.beforeUpdateNodes = beforeUpdateNodesEntry;
    }

    void Scene::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_scene_init(&mScene);
        setupCallbacks();

        String lightingTechnique = helper.getString(L"LightingTechnique", "none");
        if (lightingTechnique != "none")
        {
            try
            {
                if (lightingTechnique == "TBO")
                {
                    /* default name of lighting buffer is scene name + "LightingBufferObject" */
                    mLightingParamsBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<TextureBuffer>(getName() + "_lightingBufferObject",
                        "{\"BufferFormat\": \"RGBA32F\", \"Dynamic\": false}");
                    /* 2-bytes index, about 16k light sources support  */
                    mLightingIndexBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<TextureBuffer>(getName() + "_lightingIndexBuffer",
                        "{\"BufferFormat\": \"R32I\", \"Dynamic\": true}");
                }
                else if (lightingTechnique == "SSBO")
                {
                    /* default name of lighting buffer is scene name + "LightingBufferObject" */
                    mLightingParamsBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<SSBO>(getName() + "_lightingBufferObject",
                        "{\"Dynamic\": false}");

                    /* 2-bytes index, about 16k light sources support  */
                    mLightingIndexBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<SSBO>(getName() + "_lightingIndexBuffer",
                        "{\"Dynamic\": true}");
                }
                else if (lightingTechnique == "UBO")
                {
                    /* default name of lighting buffer is scene name + "LightingBufferObject" */
                    mLightingParamsBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<UBO>(getName() + "_lightingBufferObject",
                        "{\"Dynamic\": false}");

                    /* 2-bytes index, about 16k light sources support  */
                    mLightingIndexBuffer = getMain().getResourceManager()->
                        queryResourceFromJson<UBO>(getName() + "_lightingIndexBuffer",
                        "{\"Dynamic\": true}");
                }

                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Using lighting technique '%s' for scene %s", lightingTechnique.c_str(), getName().c_str());
            }
            catch(std::exception &ex)
            {
                LITE3D_THROW("Failed to setup lighting technique '" << lightingTechnique << "' for scene '" << getName() 
                    << "', " << ex.what());
            }
        }

        setupCameras(helper.getObjects(L"Cameras"));
        setupObjects(helper.getObjects(L"Objects"), nullptr);
    }

    void Scene::unloadImpl()
    {
        /* release lighting technique buffers */
        if (mLightingParamsBuffer)
        {
            getMain().getResourceManager()->releaseResource(getName() + "_lightingBufferObject");
            mLightingParamsBuffer = nullptr;
        }

        if (mLightingIndexBuffer)
        {
            getMain().getResourceManager()->releaseResource(getName() + "_lightingIndexBuffer");
            mLightingIndexBuffer = nullptr;
        }

        detachAllCameras();
        removeAllObjects();
        lite3d_scene_purge(&mScene);
    }

    SceneObject *Scene::addObject(const String &name, const String &templatePath, 
        SceneObject *parent)
    {
        if(mObjects.find(name) != mObjects.end())
            LITE3D_THROW(name << " make object failed.. already exist");

        SceneObject::Ptr sceneObject = createObject(name, parent);
        sceneObject->loadFromTemplate(templatePath);
        mObjects.emplace(name, sceneObject);
        return sceneObject.get();
    }

    SceneObject *Scene::addObject(const String &name, const ConfigurationReader &conf, 
        SceneObject *parent)
    {
        if(mObjects.find(name) != mObjects.end())
            LITE3D_THROW(name << " make object failed.. already exist");

        SceneObject::Ptr sceneObject = createObject(name, parent);
        sceneObject->loadFromTemplate(conf);
        mObjects.emplace(name, sceneObject);
        return sceneObject.get();
    }

    SceneObject *Scene::getObject(const String &name) const
    {
        SceneObjects::const_iterator it;
        if((it = mObjects.find(name)) != mObjects.end())
            return it->second.get();

        LITE3D_THROW(name << " object not found");
    }

    void Scene::removeAllObjects()
    {
        for (auto& object : mObjects)
        {
            object.second->detachAllNodes();
        }
        
        mObjects.clear();
    }

    void Scene::removeObject(const String &name)
    {
        SceneObjects::const_iterator it;
        if((it = mObjects.find(name)) == mObjects.end())
            LITE3D_THROW(name << " remove object failed.. not found");

        it->second->detachAllNodes();
        mObjects.erase(it);
    }
    
    void Scene::rebuildLightingBuffer()
    {
        if (!mLightingParamsBuffer || !mLightingIndexBuffer || mLights.size() == 0)
            return;

        uint32_t i = 0;

        if (mLightingParamsBuffer->bufferSizeBytes() < (mLights.size() * sizeof(lite3d_light_params)))
            mLightingParamsBuffer->extendBufferBytes(sizeof(lite3d_light_params) * mLights.size());

        for (auto &light : mLights)
        {
            mLightingParamsBuffer->setElement<lite3d_light_params>(i, &light->getLight()->getPtr()->params);
            light->getLight()->index(i++);
        }
    }
    
    void Scene::validateLightingBuffer(const Camera &camera)
    {
        if (!mLightingParamsBuffer || !mLightingIndexBuffer || mLights.size() == 0)
            return;

        // check index buffer size, extend it if needed
        if (mLightingIndexBuffer->bufferSizeBytes() < (mLights.size()+1)*sizeof(LightsIndexesStore::value_type))
            mLightingIndexBuffer->extendBufferBytes(((mLights.size()+1)*sizeof(LightsIndexesStore::value_type))-
            mLightingIndexBuffer->bufferSizeBytes());
        mLightsIndexes.clear();
        mLightsIndexes.emplace_back(0); // reserve first index for size
        
        bool anyValidated = false;
        for (auto &light : mLights)
        {
            if (!light->getLight()->enabled())
                continue;
            
            if (light->needRecalcToWorld())
            {
                light->translateToWorld();
                light->getLight()->writeToBuffer(*mLightingParamsBuffer);
                anyValidated = true;
            }

            if (!light->frustumTest() || camera.inFrustum(*light->getLight()))
            {
                light->setVisible(true);
                mLightsIndexes.emplace_back(light->getLight()->index());
            }
            else
            {
                light->setVisible(false);
            }
        }
        
        // the first index contain indexes count, max 16k
        mLightsIndexes[0] = static_cast<int32_t>(mLightsIndexes.size()-1);
        // upload indexes
        mLightingIndexBuffer->setData(&mLightsIndexes[0], 0, mLightsIndexes.size() * sizeof(LightsIndexesStore::value_type));

        if (anyValidated)
            Material::setIntGlobalParameter(getName() + "_numLights", static_cast<int32_t>(mLights.size()));
    }
    
    SceneObject::Ptr Scene::createObject(const String &name, SceneObject *parent)
    {
        return std::shared_ptr<SceneObject>(new SceneObject(name, parent, this, &getMain()));
    }

    void Scene::addLightSource(LightSceneNode *node)
    {
        mLights.emplace(node);
        rebuildLightingBuffer();
    }

    void Scene::removeLightSource(LightSceneNode *node)
    {
        mLights.erase(node);
        rebuildLightingBuffer();
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
            Camera *camera = nullptr;
            if ((camera = getMain().getCamera(cameraJson.getString(L"Name"))) == nullptr)
                camera = getMain().addCamera(cameraJson.getString(L"Name"));

            RenderTarget *renderTarget = nullptr;

            for(const ConfigurationReader &renderTargetJson : cameraJson.getObjects(L"RenderTargets"))
            {
                String renderTargetName = renderTargetJson.getString(L"Name");
                if(renderTargetName == "Window") 
                    renderTarget = getMain().window();
                else
                {
                    renderTarget = getMain().getResourceManager()->queryResource<TextureRenderTarget>(
                        renderTargetJson.getString(L"Name"),
                        renderTargetJson.getString(L"Path"));
                }
                
                uint32_t renderFlags = 0;
                if (renderTargetJson.getBool(L"RenderOpaque", true))
                    renderFlags |= LITE3D_RENDER_OPAQUE;
                if (renderTargetJson.getBool(L"RenderBlend", true))
                    renderFlags |= LITE3D_RENDER_TRANSPARENT;
                if (renderTargetJson.getBool(L"CleanColorBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_COLOR_BUFF;
                if (renderTargetJson.getBool(L"CleanDepthBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_DEPTH_BUFF;
                if (renderTargetJson.getBool(L"CleanStencilBuffer", false))
                    renderFlags |= LITE3D_RENDER_CLEAN_STENCIL_BUFF;
                if (renderTargetJson.getBool(L"DepthTest", true))
                    renderFlags |= LITE3D_RENDER_DEPTH_TEST;
                if (renderTargetJson.getBool(L"ColorOutput", true))
                    renderFlags |= LITE3D_RENDER_COLOR_OUTPUT;
                if (renderTargetJson.getBool(L"DepthOutput", true))
                    renderFlags |= LITE3D_RENDER_DEPTH_OUTPUT;
                if (renderTargetJson.getBool(L"StencilOutput", false))
                    renderFlags |= LITE3D_RENDER_STENCIL_OUTPUT;
                if (renderTargetJson.getBool(L"RenderInstancing", false))
                    renderFlags |= LITE3D_RENDER_INSTANCING;
                if (renderTargetJson.getBool(L"OcclusionQuery", false))
                    renderFlags |= LITE3D_RENDER_OCCLUSION_QUERY;
                if (renderTargetJson.getBool(L"OcclusionCulling", false))
                    renderFlags |= LITE3D_RENDER_OCCLUSION_CULLING;
                if (renderTargetJson.getBool(L"FrustumCulling", true))
                    renderFlags |= LITE3D_RENDER_FRUSTUM_CULLING;
                if (renderTargetJson.getBool(L"CustomVisibilityCheck", false))
                    renderFlags |= LITE3D_RENDER_CUSTOM_VISIBILITY_CHECK;
                if (renderTargetJson.getBool(L"SortOpaqueToNear", false))
                    renderFlags |= LITE3D_RENDER_SORT_OPAQUE_TO_NEAR;
                if (renderTargetJson.getBool(L"SortTransparentToNear", true))
                    renderFlags |= LITE3D_RENDER_SORT_TRANSPARENT_TO_NEAR;
                if (renderTargetJson.getBool(L"SortOpaqueFromNear", false))
                    renderFlags |= LITE3D_RENDER_SORT_OPAQUE_FROM_NEAR;
                if (renderTargetJson.getBool(L"SortTransparentFromNear", false))
                    renderFlags |= LITE3D_RENDER_SORT_TRANSPARENT_FROM_NEAR;

                RenderTarget::RenderLayers layers;
                auto colorLayer = renderTargetJson.getInt(L"ColorLayer", -1);
                auto depthLayer = renderTargetJson.getInt(L"DepthLayer", -1);
                if (colorLayer >= 0)
                {
                    layers.emplace_back(lite3d_framebuffer_layer { LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER, colorLayer });
                }

                if (depthLayer >= 0)
                {
                    layers.emplace_back(lite3d_framebuffer_layer { LITE3D_FRAMEBUFFER_USE_DEPTH_BUFFER, depthLayer });
                }
        
                renderTarget->addCamera(camera, this, renderTargetJson.getInt(L"TexturePass"), layers,
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

    int Scene::beginDrawBatchEntry(struct lite3d_scene *scene, 
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

    void Scene::nodeInFrustumEntry(struct lite3d_scene *scene, 
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

    void Scene::nodeOutOfFrustumEntry(struct lite3d_scene *scene, 
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

    int Scene::customVisibilityCheckEntry(struct lite3d_scene *scene, 
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
            LITE3D_EXT_OBSERVER_NOTIFY_CHECK_6(reinterpret_cast<Scene *>(scene->userdata), customVisibilityCheck, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<SceneNode *>(node->userdata),
                meshChunk,
                reinterpret_cast<Material *>(material->userdata),
                boundingVol,
                reinterpret_cast<Camera *>(camera->userdata));
            LITE3D_EXT_OBSERVER_RETURN;
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }

        return LITE3D_FALSE;
    }

    void Scene::beforeUpdateNodesEntry(struct lite3d_scene *scene, struct lite3d_camera *camera)
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

    int Scene::beginSceneRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera)
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

    void Scene::endSceneRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera)
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

    void Scene::beginOpaqueStageRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beginOpaqueStageRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::beginBlendingStageRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera)
    {
        SDL_assert(scene->userdata);
        SDL_assert(camera->userdata);

        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_2(reinterpret_cast<Scene *>(scene->userdata), beginBlendingStageRender, 
                reinterpret_cast<Scene *>(scene->userdata),
                reinterpret_cast<Camera *>(camera->userdata));
        }
        catch(std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    void Scene::attachCamera(Camera* camera, SceneObject *parent)
    {
        auto scene = camera->getScene();
        if (scene)
        {
            LITE3D_THROW("Camera '" << camera->getName() << "' already attached to scene " << scene->getName());
        }

        /* attach node to scene */
        if (!lite3d_scene_add_node(getPtr(), &camera->getPtr()->cameraNode, 
            parent ? parent->getRoot()->getPtr() : &getPtr()->rootNode))
        {
            LITE3D_THROW("Camera '" << camera->getName() << "' failed to attach to scene " << getName());
        }

        mCameras.emplace(camera->getName(), camera);
    }

    void Scene::detachCamera(Camera* camera)
    {
        auto scene = camera->getScene();
        if (scene != this)
        {
            LITE3D_THROW("Camera '" << camera->getName() << "' attached to another scene " << scene->getName());
        }

        if (!lite3d_scene_remove_node(getPtr(), &camera->getPtr()->cameraNode))
            LITE3D_THROW("Camera '" << camera->getName() << "' failed to detach from scene " << getName());

        mCameras.erase(camera->getName());
    }

    void Scene::detachAllCameras()
    {
        for (auto &camera: mCameras)
        {
            lite3d_scene_remove_node(getPtr(), &camera.second->getPtr()->cameraNode);
        }

        mCameras.clear();
    }
}

