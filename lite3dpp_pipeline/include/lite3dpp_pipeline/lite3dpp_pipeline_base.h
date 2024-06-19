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

#include <lite3dpp_pipeline/lite3dpp_bloom.h>
#include <lite3dpp_pipeline/lite3dpp_shadow_manager.h>
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    class LITE3DPP_PIPELINE_EXPORT PipelineBase : public ConfigurableResource, 
        public LifecycleObserver, public Noncopiable
    {
    public:

        PipelineBase(const String &name, const String &path, Main *main);
        virtual ~PipelineBase();

        Scene &getMainScene();
        Scene *getSkyBoxScene();
        ShadowManager *getShadowManager();
        Camera &getMainCamera();

        void setGamma(float gamma);
        void setContrast(float contrast);
        void setSaturation(float saturation);
        void setSkyBoxEmission(float emission);

    protected:

        void loadFromConfigImpl(const ConfigurationReader &helper) override;
        void unloadImpl() override;
        void timerTick(lite3d_timer *timerid) override;
        void frameBegin() override;

        virtual void createMainScene(const String& name, const String &sceneConfig);
        virtual void constructShadowManager(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator);
        virtual void constructCameraDepthPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator);
        virtual void constructCameraPipeline(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator);
        virtual void constructBloomPass(const ConfigurationReader &pipelineConfig, const String &cameraName);
        virtual void constructPostProcessPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator);
        virtual void constructSkyBoxPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
            const ConfigurationWriter &mainCameraConfig);
        
        void createBigTriangleMesh();
        void updateExposure();

    protected:

        Scene *mMainScene = nullptr;
        Scene *mSkyBoxStage = nullptr;
        Scene *mPostProcessStage = nullptr;
        String mShaderPackage;
        std::unique_ptr<ShadowManager> mShadowManager;
        std::unique_ptr<BloomEffect> mBloomEffect;
        Camera *mMainCamera = nullptr;
        RenderTarget *mDepthPass = nullptr;
        RenderTarget *mCombinePass = nullptr;
        Texture *mDepthTexture = nullptr;
        Texture *mCombinedTexture = nullptr;
        Material *mPostProcessStageMaterial = nullptr;
        Material *mSkyBoxStageMaterial = nullptr;
        stl<String>::list mResourcesList;
        PixelsData mBloomPixels;

        float mRandomSeed;
        float mExposureMax = 1.0;
        float mExposureMin = 1.0;
        float mExposureBase = 1.0;
        bool mDynamicExposureEnabled = false;
    };
}}
