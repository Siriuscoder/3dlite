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
#include <lite3dpp_pipeline/lite3dpp_pipeline_base.h>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    PipelineBase::PipelineBase(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::PIPELINE)
    {}

    Scene &PipelineBase::getMainScene()
    {
        SDL_assert(mMainScene);
        return *mMainScene;
    }

    Scene *PipelineBase::getSkyBoxScene()
    {
        return mSkyBox;
    }

    ShadowManager* PipelineBase::getShadowManager()
    {
        if (mShadowManager)
        {
            return mShadowManager.get();
        }

        return nullptr;
    }

    Camera &PipelineBase::getMainCamera()
    {
        SDL_assert(mMainCamera);
        return *mMainCamera;
    }

    void PipelineBase::loadFromConfigImpl(const ConfigurationReader &pipelineConfig)
    {
        String mainCameraName;
        mShaderPackage = pipelineConfig.getString(L"ShaderPackage");
        auto lightingTechnique = pipelineConfig.getString(L"LightingTechnique");
        auto mainScenePath = pipelineConfig.getString(L"MainScenePath");
        auto sceneJsonData = getMain().getResourceManager()->loadFileToMemory(mainScenePath);
        ConfigurationWriter sceneGeneratedConfig(static_cast<const char *>(sceneJsonData->fileBuff), sceneJsonData->fileSize);
        ConfigurationReader sceneConfig(static_cast<const char *>(sceneJsonData->fileBuff), sceneJsonData->fileSize);

        if (!lightingTechnique.empty())
        {
            sceneGeneratedConfig.set(L"LightingTechnique", lightingTechnique);
        }

        if (!sceneConfig.has(L"Cameras"))
        {
            LITE3D_THROW("Pipeline " << getName() << ": 'Cameras' section does not found");
        }

        // Создание "Полноэкранного треугольника" для использования в служебных сценах Postprocess, SSAO, и тд.
        createBigTriangleMesh();

        SceneGenerator mainSceneGenerator(getName() + "_MainScene");
        for (const auto &cameraConfig : sceneConfig.getObjects(L"Cameras"))
        {
            ConfigurationWriter cameraPipelineConfig;
            auto cameraName = cameraConfig.getString(L"Name");
            if (mainCameraName.empty())
            {
                mainCameraName = mainSceneGenerator.getName() + "_" + cameraName;
            }

            cameraPipelineConfig
                .set(L"Position", cameraConfig.getVec3(L"Position"))
                .set(L"LookAt", cameraConfig.getVec3(L"LookAt"));

            if (cameraConfig.has(L"Perspective"))
            {
                cameraPipelineConfig.set(L"Perspective", ConfigurationWriter()
                    .set(L"Znear", cameraConfig.getObject(L"Perspective").getDouble(L"Znear"))
                    .set(L"Zfar", cameraConfig.getObject(L"Perspective").getDouble(L"Zfar"))
                    .set(L"Fov", cameraConfig.getObject(L"Perspective").getDouble(L"Fov")));
            }
            else if (cameraConfig.has(L"Ortho"))
            {
                cameraPipelineConfig.set(L"Ortho", ConfigurationWriter()
                    .set(L"Near", cameraConfig.getObject(L"Ortho").getDouble(L"Near"))
                    .set(L"Far", cameraConfig.getObject(L"Ortho").getDouble(L"Far"))
                    .set(L"Left", cameraConfig.getObject(L"Ortho").getDouble(L"Left"))
                    .set(L"Right", cameraConfig.getObject(L"Ortho").getDouble(L"Right"))
                    .set(L"Bottom", cameraConfig.getObject(L"Ortho").getDouble(L"Bottom"))
                    .set(L"Top", cameraConfig.getObject(L"Ortho").getDouble(L"Top")));
            }
            else
            {
                LITE3D_THROW("Pipeline " << getName() << ": Main camera has incorrect configuration");
            }

            mainSceneGenerator.addCamera(cameraName, cameraPipelineConfig);
            constructShadowManager(pipelineConfig, cameraName, mainSceneGenerator);
            constructCameraDepthPass(pipelineConfig, cameraName, mainSceneGenerator);
            constructCameraPipeline(pipelineConfig, cameraName, mainSceneGenerator);
            break;
        }

        /* Создание главной сцены в самом конце, все остальные обьекты должны быть созданы до этого момента */
        createMainScene(mainSceneGenerator.getName(), mainSceneGenerator.generateFromExisting(sceneGeneratedConfig).write());
        mMainCamera = getMain().getCamera(mainCameraName);

        if (mShadowManager)
        {
            SDL_assert(mMainScene);
            mMainScene->addObserver(mShadowManager.get());
        }
    }

    void PipelineBase::unloadImpl()
    {
        if (mMainScene)
        {
            getMain().getResourceManager()->releaseResource(mMainScene->getName());
            mMainScene = nullptr;
        }

        if (mSkyBox)
        {
            getMain().getResourceManager()->releaseResource(mSkyBox->getName());
            mSkyBox = nullptr;
        }

        if (mDepthPass)
        {
            getMain().getResourceManager()->releaseResource(mDepthPass->getName());
            mDepthPass = nullptr;
        }

        if (mDepthTexture)
        {
            getMain().getResourceManager()->releaseResource(mDepthTexture->getName());
            mDepthTexture = nullptr;
        }

        mShadowManager.reset();
        mBloomEffect.reset();
    }

    void PipelineBase::createBigTriangleMesh()
    {
        getMain().getResourceManager()->queryResourceFromJson<Mesh>("BigTriangle.mesh",
            ConfigurationWriter().set(L"Model", "BigTriangle").set(L"Dynamic", false).write());
    }

    void PipelineBase::constructCameraPipeline(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {}

    void PipelineBase::constructCameraDepthPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        auto depthTextureName = getName() + "_" + cameraName + "_mainDepth.texture";
        ConfigurationWriter depthTextureConfig;
        depthTextureConfig.set(L"TextureType", "2D")
            .set(L"Filtering", "None")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "DEPTH");

        mDepthTexture = getMain().getResourceManager()->queryResourceFromJson<TextureImage>(depthTextureName, 
            depthTextureConfig.write());

        ConfigurationWriter depthPassConfig;
        depthPassConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::MainDepth))
            .set(L"CleanColorBuf", false)
            .set(L"CleanDepthBuf", true)
            .set(L"CleanStencilBuf", false)
            .set(L"DepthAttachments", ConfigurationWriter()
                .set(L"TextureName", depthTextureName));

        mDepthPass = getMain().getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_DepthPass", depthPassConfig.write());

        ConfigurationWriter depthPassGeneratedConfig;
        depthPassGeneratedConfig
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::DepthBuildStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::Depth))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", false)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true);

        depthPassGeneratedConfig.set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true));
        if (pipelineConfig.getBool(L"OcclusionCulling", true))
        {
            depthPassGeneratedConfig.set(L"OcclusionQuery", true)
                .set(L"SortOpaqueFromNear", true)
                .set(L"RenderInstancing", false);
        }

        sceneGenerator.addRenderTarget(cameraName, mDepthPass->getName(), depthPassGeneratedConfig);
    }

    void PipelineBase::constructShadowManager(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        /* ShadowManager создается только один раз на пайплайн, не важно сколько камер  */
        if (mShadowManager || !pipelineConfig.has(L"ShadowMaps"))
        {
            return;
        }

        mShadowManager = std::make_unique<ShadowManager>(getMain(), getName(), pipelineConfig);
        sceneGenerator.addRenderTarget(cameraName, mShadowManager->getShadowPass().getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::ShadowBuildStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::Shadow))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", false)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true)
            .set(L"CustomVisibilityCheck", true)
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }
    
    void PipelineBase::constructBloomPass(const ConfigurationReader &pipelineConfig, const String &cameraName)
    {
        if (pipelineConfig.has(L"BLOOM"))
        {
            mBloomEffect = std::make_unique<BloomEffect>(getMain(), getName(), cameraName, pipelineConfig);
        }
    }

    void PipelineBase::createMainScene(const String& name, const String &sceneConfig)
    {
        mMainScene = getMain().getResourceManager()->queryResourceFromJson<Scene>(name, sceneConfig);
    }
}}
