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

#include <ctime>
#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    PipelineBase::PipelineBase(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::PIPELINE)
    {
        std::srand(std::time(nullptr));
        mRandomSeed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        main->addObserver(this);
    }

    PipelineBase::~PipelineBase()
    {
        // В случае если загрузка пайплайна не прошла, удаляем что успело загрузиться в деструкторе
        unloadImpl();
        getMain().removeObserver(this);
    }

    Scene &PipelineBase::getMainScene()
    {
        SDL_assert(mMainScene);
        return *mMainScene;
    }

    IBLDiffuseIrradiance *PipelineBase::getIBL()
    {
        if (mIBL)
        {
            return mIBL.get();
        }

        return nullptr;
    }

    IBLMultiProbe *PipelineBase::getIBLM()
    {
        if (mIBLM)
        {
            return mIBLM.get();
        }

        return nullptr;
    }

    Scene *PipelineBase::getSkyBoxScene()
    {
        return mSkyBoxStage;
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

    void PipelineBase::setGamma(float gamma)
    {
        mPostProcessStageMaterial->setFloatParameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), "Gamma", gamma);
    }

    void PipelineBase::setContrast(float contrast)
    {
        mPostProcessStageMaterial->setFloatParameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), "Contrast", contrast);
    }

    void PipelineBase::setSaturation(float saturation)
    {
        mPostProcessStageMaterial->setFloatParameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), "Saturation", 
            saturation);
    }

    void PipelineBase::setSkyBoxEmission(float emission)
    {
        if (mSkyBoxStageMaterial)
        {
            mSkyBoxStageMaterial->setFloatParameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), "EmissionStrength",
                emission);

            if (mIBL)
            {
                mSkyBoxStageMaterial->setFloatParameter(static_cast<uint16_t>(TexturePassTypes::EnvironmentPass), "EmissionStrength",
                emission);
            }
        }
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
            constructIBL(pipelineConfig, cameraName, mainSceneGenerator);
            constructIBLM(pipelineConfig, cameraName, mainSceneGenerator);
            constructCameraPipeline(pipelineConfig, cameraName, mainSceneGenerator);
            constructPostProcessPass(pipelineConfig, cameraName, mainSceneGenerator);
            constructSkyBoxPass(pipelineConfig, cameraName, cameraPipelineConfig);
            break;
        }

        /* Создание главной сцены в самом конце, все остальные обьекты должны быть созданы до этого момента */
        createMainScene(mainSceneGenerator.getName(), mainSceneGenerator.generateFromExisting(sceneGeneratedConfig).write());
        mResourcesList.emplace_back(mMainScene->getName());
        mMainCamera = getMain().getCamera(mainCameraName);

        if (mShadowManager)
        {
            SDL_assert(mMainScene);
            mMainScene->addObserver(mShadowManager.get());
        }

        if (mIBL)
        {
            mIBL->setMainCamera(mMainCamera);
        }

        /* Для обновления параметров шейдеров */
        mMainScene->addObserver(this);

        // optimize: window clean not needed, because all pixels in last render target always will be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
    }

    void PipelineBase::unloadImpl()
    {
        for (auto it = mResourcesList.rbegin(); it != mResourcesList.rend(); ++it)
        {
            getMain().getResourceManager()->releaseResource(*it);
        }

        mResourcesList.clear();
        mShadowManager.reset();
        mBloomEffect.reset();
        mIBL.reset();
        mIBLM.reset();
    }

    void PipelineBase::createBigTriangleMesh()
    {
        if (!getMain().getResourceManager()->resourceExists("BigTriangle.mesh"))
        {
            getMain().getResourceManager()->queryResourceFromJson<Mesh>("BigTriangle.mesh",
                ConfigurationWriter().set(L"Model", "BigTriangle").set(L"Dynamic", false).write());
        }
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
        mResourcesList.emplace_back(mDepthTexture->getName());

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
        mResourcesList.emplace_back(mDepthPass->getName());

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
        mShadowManager->initialize(getName(), pipelineConfig.getString(L"ShaderPackage"));

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
            mBloomEffect->initialize();
        }
    }

    void PipelineBase::constructPostProcessPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget("Window", ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::PostProcessStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", false)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));
            
        mPostProcessStage = getMain().getResourceManager()->queryResourceFromJson<Scene>(
            getName() + "_" + cameraName + "_PostProcessStage", stageGenerator.generate().write());
        mResourcesList.emplace_back(mPostProcessStage->getName());

        SDL_assert(mCombinedTexture);

        ConfigurationWriter postProcessMaterialConfig;
        auto postProcessConfig = pipelineConfig.getObject(L"PostProcess");
        stl<ConfigurationWriter>::vector postProcessMaterialUniforms;

        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "screenMatrix"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Combined")
            .set(L"TextureName", mCombinedTexture->getName())
            .set(L"Type", "sampler"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Gamma")
            .set(L"Value", postProcessConfig.getDouble(L"Gamma", 2.2f))
            .set(L"Type", "float")
            .set(L"Scope", "global"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Exposure")
            .set(L"Value", postProcessConfig.getDouble(L"Exposure", 1.0f))
            .set(L"Type", "float"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Contrast")
            .set(L"Value", postProcessConfig.getDouble(L"Contrast", 1.0f))
            .set(L"Type", "float"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Saturation")
            .set(L"Value", postProcessConfig.getDouble(L"Saturation", 1.0f))
            .set(L"Type", "float"));
        postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "ScreenResolution")
            .set(L"Value", kmVec3 { static_cast<float>(getMain().window()->width()), 
                static_cast<float>(getMain().window()->height()), 0.0f })
            .set(L"Type", "v3"));

        if (mBloomEffect)
        {
            postProcessMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "Bloom")
                .set(L"Type", "sampler")
                .set(L"TextureName", mBloomEffect->getLastTexture().getName()));
        }
        
        postProcessMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "PostProcess.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/post_process.json"))
                .set(L"Uniforms", postProcessMaterialUniforms)
        });

        // Создаем шейдер постпроцессинга финального изображения
        mPostProcessStageMaterial = getMain().getResourceManager()->queryResourceFromJson<Material>(
            getName() + "_" + cameraName + "_PostProcessStage.material", postProcessMaterialConfig.write());
        mResourcesList.emplace_back(mPostProcessStageMaterial->getName());

        // Добавляем шейдер постпроцессинга финального изображения 
        mPostProcessStage->addObject("PostProcessBigTri", 
            BigTriObjectGenerator(mPostProcessStageMaterial->getName()).generate());

        if (postProcessConfig.has(L"DynamicExposure"))
        {
            mDynamicExposureEnabled = true;
            auto dynamicExposureConfig = postProcessConfig.getObject(L"DynamicExposure");
            mExposureMax = dynamicExposureConfig.getDouble(L"ExposureMax", 1.0);
            mExposureMin = dynamicExposureConfig.getDouble(L"ExposureMin", 1.0);
            mExposureBase = dynamicExposureConfig.getDouble(L"ExposureBase", 1.0);
        }
    }

    void PipelineBase::createSkyBoxMesh()
    {
        // Создаем built-in Skybox mesh, если еще не создан 
        if (!getMain().getResourceManager()->resourceExists("SkyBox.mesh"))
        {
            getMain().getResourceManager()->queryResourceFromJson<Mesh>("SkyBox.mesh",
                ConfigurationWriter().set(L"Model", "Skybox")
                    .set(L"Dynamic", false)
                    .set(L"Size", kmVec3 { 2.0f, 2.0f, 2.0f})
                    .write());
        }
    }

    void PipelineBase::constructSkyBoxPass(const ConfigurationReader &pipelineConfig, const String &cameraName, 
        const ConfigurationWriter &mainCameraConfig)
    {
        if (!pipelineConfig.has(L"SkyBox"))
        {
            return;
        }

        createSkyBoxMesh();

        SDL_assert(mCombinePass);

        // Нужно чтобы название камеры вышло такое же как на главной сцене (так как камера должна быть общая)
        SceneGenerator stageGenerator(getName() + "_MainScene");
        auto mainConfCopy = mainCameraConfig;
        stageGenerator.addCamera(cameraName, mainConfCopy);
        stageGenerator.addRenderTarget(cameraName, mCombinePass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::SkyBoxStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));

        ConfigurationWriter skyBoxMaterialConfig;
        auto skyBoxConfig = pipelineConfig.getObject(L"SkyBox"); 
        stl<ConfigurationWriter>::vector passes {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "SkyBox.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/skybox.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "screenMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "EmissionStrength")
                        .set(L"Type", "float")
                        .set(L"Value", skyBoxConfig.getDouble(L"EmissionStrength")),
                    ConfigurationWriter()
                        .set(L"Name", "Skybox")
                        .set(L"Type", "sampler")
                        .set(L"TextureName", getName() + "_skybox.texture")
                        .set(L"TexturePath", skyBoxConfig.getString(L"Texture")),
                })
        };

        /* Render SkyBox to environment */
        if (mIBL)
        {
            stageGenerator.addRenderTarget(cameraName, mIBL->getDiffusePass()->getName(), ConfigurationWriter()
                .set(L"Priority", static_cast<int>(RenderPassStagePriority::SkyBoxStage))
                .set(L"TexturePass", static_cast<int>(TexturePassTypes::EnvironmentPass))
                .set(L"DepthTest", true)
                .set(L"ColorOutput", true)
                .set(L"DepthOutput", false));

            ConfigurationWriter envPass;
            envPass.set(L"Pass", static_cast<int>(TexturePassTypes::EnvironmentPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "SkyBoxEnv.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/env_skybox.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "modelMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "EmissionStrength")
                        .set(L"Type", "float")
                        .set(L"Value", skyBoxConfig.getDouble(L"EmissionStrength")),
                    ConfigurationWriter()
                        .set(L"Name", "Skybox")
                        .set(L"Type", "sampler")
                        .set(L"TextureName", getName() + "_skybox.texture")
                        .set(L"TexturePath", skyBoxConfig.getString(L"Texture")),
                    ConfigurationWriter()
                        .set(L"Name", "CubeTransform")
                        .set(L"UBOName", mIBL->getViewCubeMatrixBufferName())
                        .set(L"Type", "UBO")
                });
            
            passes.emplace_back(envPass);
        }

        /* Render SkyBox to multiprobe  */
        if (mIBLM)
        {
            stageGenerator.addRenderTarget(cameraName, mIBLM->getPass()->getName(), ConfigurationWriter()
                .set(L"Priority", static_cast<int>(RenderPassStagePriority::SkyBoxStage))
                .set(L"TexturePass", static_cast<int>(TexturePassTypes::EnvironmentMultiProbePass))
                .set(L"DepthTest", true)
                .set(L"ColorOutput", true)
                .set(L"DepthOutput", false));

            ConfigurationWriter envPass;
            envPass.set(L"Pass", static_cast<int>(TexturePassTypes::EnvironmentMultiProbePass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "SkyBoxMultiProbe.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/env_skybox_multi.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "modelMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "EmissionStrength")
                        .set(L"Type", "float")
                        .set(L"Value", skyBoxConfig.getDouble(L"EmissionStrength")),
                    ConfigurationWriter()
                        .set(L"Name", "Skybox")
                        .set(L"Type", "sampler")
                        .set(L"TextureName", getName() + "_skybox.texture")
                        .set(L"TexturePath", skyBoxConfig.getString(L"Texture")),
                    ConfigurationWriter()
                        .set(L"Name", "EnvProbes")
                        .set(L"UBOName", mIBLM->getProbeBufferName())
                        .set(L"Type", "UBO"),
                    ConfigurationWriter()
                        .set(L"Name", "EnvProbesIndex")
                        .set(L"UBOName", mIBLM->getProbeIndexBufferName())
                        .set(L"Type", "UBO")
                });
            
            passes.emplace_back(envPass);
        }

        mSkyBoxStage = getMain().getResourceManager()->queryResourceFromJson<Scene>(
            getName() + "_" + cameraName + "_SkyBoxStage", stageGenerator.generate().write());
        mResourcesList.emplace_back(mSkyBoxStage->getName());

        skyBoxMaterialConfig.set(L"Passes", passes);

        // Создаем шейдер skybox
        mSkyBoxStageMaterial = getMain().getResourceManager()->queryResourceFromJson<Material>(
            getName() + "_" + cameraName + "_SkyBoxStage.material", skyBoxMaterialConfig.write());
        mResourcesList.emplace_back(mSkyBoxStageMaterial->getName());

        // Добавляем шейдер skybox
        mSkyBoxStage->addObject("SkyBox", SkyBoxObjectGenerator(mSkyBoxStageMaterial->getName()).generate());
    }

    void PipelineBase::constructIBL(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        if (!pipelineConfig.has(L"IBL"))
        {
            return;
        }

        createSkyBoxMesh();

        mIBL = std::make_unique<IBLDiffuseIrradiance>(getMain(), getName(), mShaderPackage);
        mIBL->initialize(pipelineConfig);

        sceneGenerator.addRenderTarget(cameraName, mIBL->getDiffusePass()->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::ForwardStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::EnvironmentPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true)
            .set(L"FrustumCulling", false)
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }

    void PipelineBase::constructIBLM(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        if (!pipelineConfig.has(L"IBLMultiProbe"))
        {
            return;
        }

        mIBLM = std::make_unique<IBLMultiProbe>(getMain(), getName());
        mIBLM->initialize(pipelineConfig);

        sceneGenerator.addRenderTarget(cameraName, mIBLM->getPass()->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::ForwardStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::EnvironmentMultiProbePass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true)
            .set(L"FrustumCulling", false)
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }

    void PipelineBase::createMainScene(const String& name, const String &sceneConfig)
    {
        mMainScene = getMain().getResourceManager()->queryResourceFromJson<Scene>(name, sceneConfig);
    }

    void PipelineBase::updateExposure()
    {
        if (!mBloomEffect || !mPostProcessStageMaterial || !mDynamicExposureEnabled)
        {
            return;
        }

        mBloomEffect->getMiddleTexture().getPixels(0, mBloomPixels);

        auto it = mBloomPixels.begin();
        kmVec3 rgbAverage = KM_VEC3_ZERO;
        for (; it != mBloomPixels.end(); )
        {
            rgbAverage.x += *reinterpret_cast<float *>(&(*it)); it += sizeof(float);
            rgbAverage.y += *reinterpret_cast<float *>(&(*it)); it += sizeof(float);
            rgbAverage.z += *reinterpret_cast<float *>(&(*it)); it += sizeof(float);
        }

        kmVec3Scale(&rgbAverage, &rgbAverage, 1.0f / (mBloomPixels.size() / (3 * sizeof(float))));
        auto exposure = mExposureBase / kmVec3Length(&rgbAverage);
        exposure = std::max(mExposureMin, std::min(mExposureMax, exposure));
        mPostProcessStageMaterial->setFloatParameter(static_cast<int>(TexturePassTypes::RenderPass), "Exposure", exposure);
    }

    void PipelineBase::timerTick(lite3d_timer *timerid)
    {
        updateExposure();
    }

    bool PipelineBase::beginSceneRender(Scene *scene, Camera *camera)
    {
        Material::setFloatv3GlobalParameter("Eye", getMainCamera().getWorldPosition());
        return true;
    }
}}
