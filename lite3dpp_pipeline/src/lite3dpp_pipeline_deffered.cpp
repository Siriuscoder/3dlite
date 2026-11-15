/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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

    PipelineDeffered::PipelineDeffered(const String &name, const String &path, Main &main) : 
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
        if (mLightComputeShader && mSSAOPass)
        {
            mLightComputeShader->getShaderParameters().setIntParameter("AOEnabled", flag);
            flag ? mSSAOPass->enable() : mSSAOPass->disable();
        }
    }

    void PipelineDeffered::setSkyBoxEmission(float emission)
    {
        PipelineBase::setSkyBoxEmission(emission);
        if (mLightComputeShader && getConfig().has(L"SkyBox"))
        {
            mLightComputeShader->getShaderParameters().setFloatParameter("EmissionStrength", emission);
        }
    }

    void PipelineDeffered::constructSkyBoxPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        const ConfigurationWriter &mainCameraConfig)
    {
        // The compute lighting pass draws the screen skybox; keep geometry only for GI captures.
        createSkyBoxStage(pipelineConfig, cameraName, mainCameraConfig, false);
    }

    void PipelineDeffered::unloadImpl()
    {
        mLightComputeShader = nullptr;
        PipelineBase::unloadImpl();
        mCombinePass = nullptr;
    }

    bool PipelineDeffered::beginSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params)
    {
        if (scene == mMainScene && params->priority == static_cast<int32_t>(RenderPassStagePriority::BlendDecalStage))
        {
            const auto &mainCamera = getMainCamera();
            ShaderParameters::setFloatm4GlobalParameter("CameraView", mainCamera.getViewMatrix());
            ShaderParameters::setFloatm4GlobalParameter("CameraProjection", mainCamera.getProjMatrix());
            ShaderParameters::setFloatv3GlobalParameter("Eye", mainCamera.getWorldPosition());
            ShaderParameters::setIntGlobalParameter("FrameNumber", static_cast<int32_t>(getMain().getRenderStats()->framesCount));

            // The shader writes the background for pixels without geometry or skybox.
            const auto &background = mCombinePass->getPtr()->cleanColor;
            mLightComputeShader->getShaderParameters().setFloatv3Parameter("ClearColor",
                kmVec3 { background.x, background.y, background.z });
            // lighting.comp assigns one workgroup to each output pixel.
            const uint32_t groupsXCount = (mCombinedTexture->getWidth() + lightComputeLocalThreadsX - 1) / lightComputeLocalThreadsX;
            const uint32_t groupsYCount = (mCombinedTexture->getHeight() + lightComputeLocalThreadsY - 1) / lightComputeLocalThreadsY;
            mLightComputeShader->dispatch(groupsXCount, groupsYCount, 1);
        }

        // Copy the opaque combined texture only after compute lighting has finished.
        return PipelineBase::beginSceneRender(scene, camera, params);
    }

    void PipelineDeffered::constructLightComputePass(const ConfigurationReader &pipelineConfig)
    {
        SDL_assert(mMainScene);
        SDL_assert(mCombinePass);

        ConfigurationWriter lightComputeConfig;
        stl<ConfigurationWriter>::vector lightComputeMaterialUniforms;

        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "outputImage")
            .set(L"Type", "imageStore")
            .set(L"Direction", "output")
            .set(L"TextureName", mCombinedTexture->getName()));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "GBuffer")
            .set(L"TextureName", mGBufferTexture->getName())
            .set(L"Type", "sampler"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "ClearColor")
            .set(L"Type", "v3"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "Eye")
            .set(L"Scope", "global")
            .set(L"Type", "v3"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "CameraView")
            .set(L"Type", "m4")
            .set(L"Scope", "global"));
        lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
            .set(L"Name", "CameraProjection")
            .set(L"Type", "m4")
            .set(L"Scope", "global"));

        if (pipelineConfig.has(L"SkyBox"))
        {
            const auto skyBoxConfig = pipelineConfig.getObject(L"SkyBox");
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "Skybox")
                .set(L"TextureName", getName() + "_skybox.texture")
                .set(L"TexturePath", skyBoxConfig.getString(L"Texture"))
                .set(L"Type", "sampler"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "EmissionStrength")
                .set(L"Value", skyBoxConfig.getDouble(L"EmissionStrength"))
                .set(L"Type", "float"));
            ShaderProgram::addGlobalDefinition("LITE3D_LIGHT_PASS_SKYBOX", "1");
        }

        /* IBL present, using env probe */
        if (mGIProbeManager)
        {
            // according to MultiRender case EnvironmentProbe texture being declared as bindless texture
            if (!pipelineConfig.getBool(L"MultiRender", false))
            {
                lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                    .set(L"Name", "EnvironmentProbe")
                    .set(L"TextureName", mGIProbeManager->getEnvProbeTexture()->getName())
                    .set(L"Type", "sampler"));
            }

            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "EnvProbesData")
                .set(L"SSBOName",  mGIProbeManager->getProbeBufferName())
                .set(L"Type", "SSBO"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "BrdfLUT")
                .set(L"TextureName", mGIProbeManager->getBrdfLUTName())
                .set(L"Type", "sampler"));
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
                .set(L"Name", "MomentsMaps")
                .set(L"TextureName", mShadowManager->getMomentsMapTexture()->getName())
                .set(L"Type", "sampler"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "ShadowMatrix")
                .set(L"UBOName", mShadowManager->getShadowMatrixBuffer()->getName())
                .set(L"Type", "UBO"));
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

        if (pipelineConfig.getBool(L"ComputeAreaLighting", false))
        {
            SDL_assert(mLTCLut01);
            SDL_assert(mLTCLut02);

            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "ltcLut1")
                .set(L"TextureName", mLTCLut01->getName())
                .set(L"Type", "sampler"));
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "ltcLut2")
                .set(L"TextureName", mLTCLut02->getName())
                .set(L"Type", "sampler"));
        }

        if (pipelineConfig.getBool(L"MultiRender", false))
        {
            lightComputeMaterialUniforms.emplace_back(ConfigurationWriter()
                .set(L"Name", "MultiRenderMaterialDataBuffer")
                .set(L"SSBOName", MultiRenderMaterial::MultiRenderMaterialDataBufferName.data())
                .set(L"Type", "SSBO"));
        }

        lightComputeConfig.set(L"Program", ConfigurationWriter()
                .set(L"Name", "LightCompute.program")
                .set(L"Path", mShaderPackage + ":shaders/json/lightpass.json"))
            .set(L"Uniforms", lightComputeMaterialUniforms);

        auto maxLightComputeThreads = ShaderProgram::getOptimalWorkGroupSize(mCombinedTexture->getWidth(), 
            mCombinedTexture->getHeight());
        lightComputeLocalThreadsX = maxLightComputeThreads[0];
        lightComputeLocalThreadsY = maxLightComputeThreads[1];

        ShaderProgram::addGlobalDefinition("LITE3D_LIGHT_COMPUTE_THREADS_X", std::to_string(lightComputeLocalThreadsX));
        ShaderProgram::addGlobalDefinition("LITE3D_LIGHT_COMPUTE_THREADS_Y", std::to_string(lightComputeLocalThreadsY));

        mLightComputeShader = getMain().getResourceManager().queryResourceFromJson<ComputeShader>(
            getName() + "_LightCompute.comp", lightComputeConfig.write(), this);
    }

    void PipelineDeffered::constructGBufferPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {
        /*
         * GBuffer layout: 4 x RGBA32F
         *
         * +-----------+-------------------+-------------------+-------------------+-------------------+
         * | Channel   | X                 | Y                 | Z                 | W                 |
         * +-----------+-------------------+-------------------+-------------------+-------------------+
         * | channel01 | iwv.x             | iwv.y             | iwv.z             | MatID(24)         |
         * | channel02 | normal.x          | normal.y          | normal.z          | emission.r        |
         * | channel03 | albedo.rg (2x12)  | albedo.b(12)+S(12)| geoNormal (Oct24) | emission.g        |
         * | channel04 | R(12)+M(12)       | AO / reserved     | reserved          | emission.b        |
         * +-----------+-------------------+-------------------+-------------------+-------------------+
         *
         * Packing:
         *   albedo.rg  : 2 x UNORM12 -> FP32
         *   albedo.b/S : 2 x UNORM12 -> FP32
         *   R/M        : 2 x UNORM12 -> FP32
         *   geoNormal  : Octahedral 2 x UNORM12 -> FP32
         *
         * S = Specular, R = Roughness, M = Metallic
         *
         * All new packed values use numeric uint-to-float conversion (no NaN).
         * Packed channels must be read without texture filtering.
         */

        ConfigurationWriter gBufferTextureConfig;
        gBufferTextureConfig.set(L"TextureType", "2D_ARRAY")
            .set(L"Filtering", "None")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RGBA")
            .set(L"InternalFormat", "RGBA32F")
            .set(L"Depth", 4);

        mGBufferTexture = getMain().getResourceManager().queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_geometry_data.texture", gBufferTextureConfig.write(), this);

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

        mGBufferPass = getMain().getResourceManager().queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_GBufferPass", gBufferTargetConfig.write(), this);

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
            .set(L"TextureFormat", "RGBA")
            .set(L"InternalFormat", "RGBA32F");

        mCombinedTexture = getMain().getResourceManager().queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_combined.texture", combinedTextureConfig.write(), this);

        SDL_assert(mDepthTexture);
        ConfigurationWriter combinedTargetConfig;
        combinedTargetConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
            .set(L"Priority", static_cast<int>(RenderPassPriority::Combine))
            .set(L"CleanColorBuf", false)
            .set(L"CleanDepthBuf", false)
            .set(L"CleanStencilBuf", false)
            .set(L"ColorAttachments", ConfigurationWriter()
                .set(L"Attachments", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter().set(L"TextureName", mCombinedTexture->getName())
                }))
            .set(L"DepthAttachments", ConfigurationWriter()
                .set(L"TextureName", mDepthTexture->getName()));

        mCombinePass = getMain().getResourceManager().queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_CombinePass", combinedTargetConfig.write(), this);

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
        // Создание SSAO текстуры
        ConfigurationWriter ssaoTextureConfig;
        ssaoTextureConfig.set(L"TextureType", "2D")
            .set(L"Filtering", "Linear")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RED")
            .set(L"InternalFormat", "R16F")
            .set(L"Scale", pipelineConfig.getObject(L"SSAO").getInt(L"FramebufferScale", 4));

        mSSAOTexture = getMain().getResourceManager().queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_SSAO.texture", ssaoTextureConfig.write(), this);

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

        mSSAOPass = getMain().getResourceManager().queryResourceFromJson<TextureRenderTarget>(
            getName() + "_" + cameraName + "_SSAOPass", ssaoTargetConfig.write(), this);

        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget(mSSAOPass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::SSAOBuildStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", false)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));
            
        mSSAOStage = getMain().getResourceManager().queryResourceFromJson<Scene>(cameraName + "_SSAOStage",
            stageGenerator.generate().write(), this);

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
                        .set(L"Name", "CameraView")
                        .set(L"Type", "m4")
                        .set(L"Scope", "global"),
                    ConfigurationWriter()
                        .set(L"Name", "CameraProjection")
                        .set(L"Type", "m4")
                        .set(L"Scope", "global")
                })
        });

        // Создаем служебный шейдер отвечающий за расчет SSAO
        mSSAOStageMaterial = getMain().getResourceManager().queryResourceFromJson<Material>(
            getName() + "_" + cameraName + "_SSAOStage.material", ssaoMaterialConfig.write(), this);

        // Добавляем шейдер расчета SSAO
        mSSAOStage->addObject("SSAOBigTri", BigTriObjectGenerator(mSSAOStageMaterial->getName()).generate());
    }
}}
