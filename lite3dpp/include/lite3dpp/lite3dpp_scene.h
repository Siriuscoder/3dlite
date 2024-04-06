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

#include <lite3d/lite3d_scene.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_scene_object.h>
#include <lite3dpp/lite3dpp_camera.h>
#include <lite3dpp/lite3dpp_light_source.h>
#include <lite3dpp/lite3dpp_observer.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Scene : public Observable<SceneObserver>, 
        public SceneObserver, public ConfigurableResource, public Noncopiable
    {
        friend LightSceneNode;
    public:

        using SceneObjects = stl<String, SceneObject::Ptr>::unordered_map;
        using SceneLights = stl<LightSceneNode *>::unordered_set;
        using SceneCameras = stl<String, Camera*>::unordered_map;
        using LightsIndexesStore = stl<int32_t>::vector;

        Scene(const String &name, 
            const String &path, Main *main);
        ~Scene();

        inline lite3d_scene *getPtr()
        { return &mScene; }

        SceneObject *addObject(const String &name, const String &templatePath, 
            SceneObject *parent = nullptr);
        SceneObject *addObject(const String &name, const ConfigurationReader &conf, 
            SceneObject *parent = nullptr);

        void attachCamera(Camera* camera, SceneObject *parent = nullptr);
        void detachCamera(Camera* camera);
        SceneObject *getObject(const String &name) const;
        inline const SceneObjects &getObjects() const
        { return mObjects; }

        void removeAllObjects();
        void removeObject(const String &name);
        void detachAllCameras();

        inline const SceneLights &getLights() const 
        { return mLights; }

        size_t usedVideoMemBytes() const override;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;
        
        void setupCallbacks();
        void rebuildLightingBuffer();
        void validateLightingBuffer(const Camera &camera);
        void addLightSource(LightSceneNode *node);
        void removeLightSource(LightSceneNode *node);
        
        virtual SceneObject::Ptr createObject(const String &name, SceneObject *parent);

    private:

        void setupObjects(const stl<ConfigurationReader>::vector &objects, SceneObject *base);
        void setupCameras(const stl<ConfigurationReader>::vector &cameras);

        static int beginDrawBatchEntry(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, struct lite3d_material *material);

        static void nodeInFrustumEntry(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol, 
            struct lite3d_camera *camera);

        static void nodeOutOfFrustumEntry(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
            struct lite3d_camera *camera);

        static int customVisibilityCheckEntry(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
            struct lite3d_camera *camera);

        static void beforeUpdateNodesEntry(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static int beginSceneRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void endSceneRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void beginOpaqueStageRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void beginBlendingStageRenderEntry(struct lite3d_scene *scene, struct lite3d_camera *camera);

        lite3d_scene mScene;
        SceneObjects mObjects;
        SceneLights mLights;
        SceneCameras mCameras;
        BufferBase *mLightingParamsBuffer;
        BufferBase *mLightingIndexBuffer;
        LightsIndexesStore mLightsIndexes;
    };
}

