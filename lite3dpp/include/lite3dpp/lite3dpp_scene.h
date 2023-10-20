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
#include <lite3dpp/lite3dpp_scene_object.h>
#include <lite3dpp/lite3dpp_camera.h>
#include <lite3dpp/lite3dpp_light_source.h>
#include <lite3dpp/lite3dpp_observer.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Scene : public Observable<SceneObserver>, public ConfigurableResource, public Noncopiable
    {
    public:

        typedef stl<String, SceneObject::Ptr>::unordered_map Objects;
        typedef stl<String, LightSceneNode *>::unordered_map Lights;
        typedef stl<String, Camera*>::unordered_map Cameras;
        typedef stl<lite3d_light_params>::vector LightsStore;
        typedef stl<int32_t>::vector LightsIndexesStore;


        Scene(const String &name, 
            const String &path, Main *main);
        ~Scene();

        inline lite3d_scene *getPtr()
        { return &mScene; }

        SceneObject *addObject(const String &name,
            const String &templatePath, SceneObject *parent);
        void attachCamera(Camera* camera, SceneObject *parent);
        void detachCamera(Camera* camera);
        SceneObject *getObject(const String &name) const;
        inline const Objects &getObjects() const
        { return mObjects; }

        void removeAllObjects();
        void removeObject(const String &name);
        void detachAllCameras();
        
        LightSceneNode *addLightNode(LightSceneNode *light);
        LightSceneNode *getLightNode(const String &name) const;
        void removeLight(const String &name);
        void removeAllLights();

        inline const Lights &getLights() const 
        { return mLights; }

        size_t usedVideoMemBytes() const override;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;
            
        void rebuildLightingBuffer();
        void validateLightingBuffer(const Camera &camera);
        
        virtual SceneObject::Ptr createObject(const String &name, SceneObject *parent);

    private:

        void setupObjects(const stl<ConfigurationReader>::vector &objects, SceneObject *base);
        void setupCameras(const stl<ConfigurationReader>::vector &cameras);

        static int beginDrawBatch(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, struct lite3d_material *material);

        static void nodeInFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol, 
            struct lite3d_camera *camera);

        static void nodeOutOfFrustum(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
            struct lite3d_camera *camera);

        static int customVisibilityCheck(struct lite3d_scene *scene, 
            struct lite3d_scene_node *node, struct lite3d_mesh_chunk *meshChunk, 
            struct lite3d_material *material, struct lite3d_bounding_vol *boundingVol,
            struct lite3d_camera *camera);

        static void beforeUpdateNodes(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static int beginSceneRender(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void endSceneRender(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void beginOpaqueStageRender(struct lite3d_scene *scene, struct lite3d_camera *camera);
        static void beginBlendingStageRender(struct lite3d_scene *scene, struct lite3d_camera *camera);

        lite3d_scene mScene;
        Objects mObjects;
        Lights mLights;
        Cameras mCameras;
        BufferBase *mLightingParamsBuffer;
        BufferBase *mLightingIndexBuffer;
        LightsIndexesStore mLightsIndexes;
    };
}

