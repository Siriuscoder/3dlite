/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <lite3dpp_pipeline/lite3dpp_pipeline_deffered.h>
#include <lite3dpp/lite3dpp_material_multi_render.h>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    PipelineDeffered::PipelineDeffered(const String &name, const String &path, Main *main) : 
        PipelineBase(name, path, main)
    {}

    void PipelineDeffered::constructCameraPipeline(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        constructGBufferPass(pipelineConfig, cameraName, sceneGenerator);
        constructSSAOPass(pipelineConfig, cameraName);
        constructCombinedPass(pipelineConfig, cameraName, sceneGenerator);
        constructBloomPass(pipelineConfig, cameraName);
    }

    void PipelineDeffered::loadFromConfigImpl(const ConfigurationReader &pipelineConfig)
    {
        PipelineBase::loadFromConfigImpl(pipelineConfig);
        /* Create the lightpass after the main scene was loaded */
        constructLightComputePass(pipelineConfig);
    }

    void PipelineDeffered::enableSSAO(bool flag)
    {
        if (mLightComputeStageMaterial && mSSAOPass)
        {
            mLightComputeStageMaterial->setIntParameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), "AOEnabled", flag);
            flag ? mSSAOPass->enable() : mSSAOPass->disable();
        }
    }

    void PipelineDeffered::constructLightComputePass(const ConfigurationReader &pipelineConfig)
    {
        SDL_assert(mMainScene);
        SDL_assert(mCombinePass);

        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget(mCombinePass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::LightComputeStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", false)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));
            
        mLightComputeStage = getMain().getResourceManager()->queryResourceFromJson<Scene>(getName() + "_LightComputeStage",
            stageGenerator.generate().write());
        mResourcesList.emplace_back(mLightComputeStage->getName());

        ConfigurationWriter lightComputeMaterialConfig;
        stl<ConfigurationWriter>::vector lightComputeMaterialUniforms;

        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "screenMatrix"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "GBuffer")
            .set(L"TextureName", mGBufferTexture->getName())
            .set(L"Type", "sampler"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Eye")
            .set(L"Scope", "global")
            .set(L"Type", "v3"));

        /* IBL present, using env probe */
        if (mIBL)
        {
            // according to MultiRender case EnvironmentProbe texture being declared as bindless texture
            if (!pipelineConfig.getBool(L"MultiRender", false))
            {
                lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                    .set(L"Name", "EnvironmentProbe")
                    .set(L"TextureName", mIBL->getEnvProbeTexture()->getName())
                    .set(L"Type", "sampler"));
            }

            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "EnvProbesData")
                .set(L"UBOName",  mIBL->getProbeBufferName())
                .set(L"Type", "UBO"));
        }
        else
        {
            if (pipelineConfig.has(L"EnvironmentTexture"))
            {
                lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                    .set(L"Name", "Environment")
                    .set(L"TextureName", "environment.texture")
                    .set(L"TexturePath", pipelineConfig.getString(L"EnvironmentTexture"))
                    .set(L"Type", "sampler"));
            }
        }
        
        auto lightingTechType = pipelineConfig.getString(L"LightingTechnique");
        if (lightingTechType == "UBO" && mMainScene->getLightParamsBuffer())
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "LightSources")
                .set(L"UBOName", mMainScene->getName() + "_lightingBufferObject")
                .set(L"Type", "UBO"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "LightIndexes")
                .set(L"UBOName", mMainScene->getName() + "_lightingIndexBuffer")
                .set(L"Type", "UBO"));
        }
        else if (lightingTechType == "SSBO" && mMainScene->getLightParamsBuffer())
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "LightSources")
                .set(L"SSBOName", mMainScene->getName() + "_lightingBufferObject")
                .set(L"Type", "SSBO"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "LightIndexes")
                .set(L"SSBOName", mMainScene->getName() + "_lightingIndexBuffer")
                .set(L"Type", "SSBO"));
        }
        
        if (mShadowManager)
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "ShadowMaps")
                .set(L"TextureName", mShadowManager->getShadowMapTexture()->getName())
                .set(L"Type", "sampler"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "ShadowMatrix")
                .set(L"UBOName", mShadowManager->getShadowMatrixBuffer()->getName())
                .set(L"Type", "UBO"));
                
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "CameraView")
                .set(L"Type", "m4"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "CameraProjection")
                .set(L"Type", "m4"));
        }

        if (mSSAOTexture)
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "AOMap")
                .set(L"TextureName", mSSAOTexture->getName())
                .set(L"Type", "sampler"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "AOEnabled")
                .set(L"Value", 1)
                .set(L"Type", "int"));
        }

        if (pipelineConfig.getBool(L"MultiRender", false))
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "MultiRenderMaterialDataBuffer")
                .set(L"SSBOName", MultiRenderMaterial::MultiRenderMaterialDataBufferName.data())
                .set(L"Type", "SSBO"));
        }

        lightComputeMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "LightCompute.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/lightpass.json"))
                .set(L"Uniforms", lightComputeMaterialUniforms)
        });
        
        // Создаем служебный шейдер отвечающий за расчет освещения в экранном пространстве
        mLightComputeStageMaterial = getMain().getResourceManager()->queryResourceFromJson<Material>(
            getName() + "_LightComputeStage.material", lightComputeMaterialConfig.write());
        mResourcesList.emplace_back(mLightComputeStageMaterial->getName());

        // Добавляем шейдер расчета освещения в экранном пространстве
        mLightComputeStage->addObject("LightComputeBigTri", 
            BigTriObjectGenerator(mLightComputeStageMaterial->getName()).generate());
    }

    void PipelineDeffered::constructGBufferPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        // Создание GeometryBuffer
        // Полноэкранная текстура состоязая из 4х слоев для записи данных о геометрии в каждой точке экрана
        ConfigurationWriter gBufferTextureConfig;
        gBufferTextureConfig.set(L"TextureType", "2D_ARRAY")
            .set(L"Filtering", "None")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RGBA")
            .set(L"InternalFormat", "RGBA32F")
            .set(L"Depth", 4);

        mGBufferTexture = getMain().getResourceManager()->queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_geometry_data.texture", gBufferTextureConfig.write());
        mResourcesList.emplace_back(mGBufferTexture->getName());

        ConfigurationWriter gBufferTargetConfig;
        stl<ConfigurationWriter>::vector gBufferColorAttachmentsConfig;
        gBufferTargetConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::GBuffer))
            .set(L"CleanColorBuf", true)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false);

        for (int i = 0; i < 4; ++i)
        {
            gBufferColorAttachmentsConfig.emplace_back(ConfigurationWriter()
                .set(L"TextureName", mGBufferTexture->getName())
                .set(L"Layer", i));
        }

        SDL_assert(mDepthTexture);
        gBufferTargetConfig.set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", gBufferColorAttachmentsConfig))
            .set(L"DepthAttachments", ConfigurationWriter()
            .set(L"TextureName", mDepthTexture->getName()));

        mGBufferPass = getMain().getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_GBufferPass", gBufferTargetConfig.write());
        mResourcesList.emplace_back(mGBufferPass->getName());

        sceneGenerator.addRenderTarget(cameraName, mGBufferPass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::GBufferBuildStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false)
            .set(L"RenderBlend", false)
            .set(L"RenderOpaque", true)
            .set(L"OcclusionCulling", pipelineConfig.getBool(L"OcclusionCulling", true))
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }

    void PipelineDeffered::constructCombinedPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        // Создание Combined текстуры
        // Содержит промежуточные результаты HDR рендера 
        ConfigurationWriter combinedTextureConfig;
        combinedTextureConfig.set(L"TextureType", "2D")
            .set(L"Filtering", "None")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RGB")
            .set(L"InternalFormat", "RGB32F");

        mCombinedTexture = getMain().getResourceManager()->queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_combined.texture", combinedTextureConfig.write());
        mResourcesList.emplace_back(mCombinedTexture->getName());

        SDL_assert(mDepthTexture);
        ConfigurationWriter combinedTargetConfig;
        combinedTargetConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::Combine))
            .set(L"CleanColorBuf", true)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false)
            .set(L"ColorAttachments", ConfigurationWriter()
                .set(L"Attachments", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter().set(L"TextureName", mCombinedTexture->getName())
                }))
            .set(L"DepthAttachments", ConfigurationWriter()
                .set(L"TextureName", mDepthTexture->getName()));

        mCombinePass = getMain().getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_CombinePass", combinedTargetConfig.write());
        mResourcesList.emplace_back(mCombinePass->getName());

        sceneGenerator.addRenderTarget(cameraName, mCombinePass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::BlendDecalStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", true)
            .set(L"RenderOpaque", false)
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }

    void PipelineDeffered::constructSSAOPass(const ConfigurationReader &pipelineConfig, const String &cameraName)
    {
        if (!pipelineConfig.has(L"SSAO"))
        {
            return;
        }

        // Создание SSAO текстуры
        ConfigurationWriter ssaoTextureConfig;
        ssaoTextureConfig.set(L"TextureType", "2D")
            .set(L"Filtering", "Linear")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RED")
            .set(L"InternalFormat", "R16F")
            .set(L"Scale", pipelineConfig.getObject(L"SSAO").getInt(L"FramebufferScale", 4));

        mSSAOTexture = getMain().getResourceManager()->queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_SSAO.texture", ssaoTextureConfig.write());
        mResourcesList.emplace_back(mSSAOTexture->getName());

        ConfigurationWriter ssaoTargetConfig;
        ssaoTargetConfig
            .set(L"Scale", pipelineConfig.getObject(L"SSAO").getInt(L"FramebufferScale", 4))
            .set(L"BackgroundColor", kmVec4 { 1.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::SSAO))
            .set(L"CleanColorBuf", true)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false)
            .set(L"ColorAttachments", ConfigurationWriter()
                .set(L"Attachments", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter().set(L"TextureName", mSSAOTexture->getName())
                }));

        mSSAOPass = getMain().getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_SSAOPass", ssaoTargetConfig.write());
        mResourcesList.emplace_back(mSSAOPass->getName());

        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget(mSSAOPass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::SSAOBuildStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", false)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));
            
        mSSAOStage = getMain().getResourceManager()->queryResourceFromJson<Scene>(cameraName + "_SSAOStage",
            stageGenerator.generate().write());
        mResourcesList.emplace_back(mSSAOStage->getName());

        SDL_assert(mGBufferTexture);

        ConfigurationWriter ssaoMaterialConfig;
        ssaoMaterialConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "SSAO.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/ssao.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "screenMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "GBuffer")
                        .set(L"TextureName", mGBufferTexture->getName())
                        .set(L"Type", "sampler"),
                    ConfigurationWriter()
                        .set(L"Name", "AORadius")
                        .set(L"Value", pipelineConfig.getObject(L"SSAO").getDouble(L"AORadius"))
                        .set(L"Type", "float"),
                    ConfigurationWriter()
                        .set(L"Name", "RandomSeed")
                        .set(L"Value", mRandomSeed)
                        .set(L"Type", "float"),
                    ConfigurationWriter()
                        .set(L"Name", "CameraView")
                        .set(L"Type", "m4"),
                    ConfigurationWriter()
                        .set(L"Name", "CameraProjection")
                        .set(L"Type", "m4")
                })
        });

        // Создаем служебный шейдер отвечающий за расчет SSAO
        mSSAOStageMaterial = getMain().getResourceManager()->queryResourceFromJson<Material>(
            getName() + "_" + cameraName + "_SSAOStage.material", ssaoMaterialConfig.write());
        mResourcesList.emplace_back(mSSAOStageMaterial->getName());

        // Добавляем шейдер расчета SSAO
        mSSAOStage->addObject("SSAOBigTri", BigTriObjectGenerator(mSSAOStageMaterial->getName()).generate());
    }

    bool PipelineDeffered::beginSceneRender(Scene *scene, Camera *camera)
    {
        auto &viewMatrix = getMainCamera().getViewMatrix();
        auto &projMatrix = getMainCamera().getProjMatrix();

        if (mSSAOStageMaterial)
        {
            mSSAOStageMaterial->setFloatm4Parameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), 
                "CameraView", viewMatrix);
            mSSAOStageMaterial->setFloatm4Parameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), 
                "CameraProjection", projMatrix);
        }

        if (mLightComputeStageMaterial)
        {
            mLightComputeStageMaterial->setFloatm4Parameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), 
                "CameraView", viewMatrix);
            mLightComputeStageMaterial->setFloatm4Parameter(static_cast<uint16_t>(TexturePassTypes::RenderPass), 
                "CameraProjection", projMatrix);
        }

        return PipelineBase::beginSceneRender(scene, camera);
    }
}}
