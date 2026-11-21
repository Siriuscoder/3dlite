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
        if (mLightComputeShader && mSSAOComputeShader)
        {
            mLightComputeShader->getShaderParameters().setIntParameter("AOEnabled", flag);
            mSSAOEnabled = flag;
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
        mSSAOComputeShader = nullptr;
        mSSAOFilterShader = nullptr;
        mSSAOTexture = nullptr;
        mSSAOTexture2 = nullptr;
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

            computeSSAO();
            computeLighting();
        }

        // Copy the opaque combined texture only after compute lighting has finished.
        return PipelineBase::beginSceneRender(scene, camera, params);
    }

    void PipelineDeffered::computeSSAO()
    {
        if (!mSSAOEnabled)
        {
            return;
        }

        SDL_assert(mSSAOComputeShader);
        SDL_assert(mSSAOFilterShader);

        const uint32_t groupsXCount = (mSSAOTexture->getWidth() + ssaoComputeLocalThreadsX - 1) / ssaoComputeLocalThreadsX;
        const uint32_t groupsYCount = (mSSAOTexture->getHeight() + ssaoComputeLocalThreadsY - 1) / ssaoComputeLocalThreadsY;
        mSSAOComputeShader->dispatch(groupsXCount, groupsYCount, 1);

        // Ping-pong filtering keeps each pass's input separate from its output.
        auto &filterParameters = mSSAOFilterShader->getShaderParameters();
        filterParameters.setIntParameter("Direction", 1);
        filterParameters.setSamplerParameter("AOMap", *mSSAOTexture);
        filterParameters.setImageStoreParameter("FilteredOutput", *mSSAOTexture2);
        mSSAOFilterShader->dispatch(groupsXCount, groupsYCount, 1);

        filterParameters.setIntParameter("Direction", 0);
        filterParameters.setSamplerParameter("AOMap", *mSSAOTexture2);
        filterParameters.setImageStoreParameter("FilteredOutput", *mSSAOTexture);
        mSSAOFilterShader->dispatch(groupsXCount, groupsYCount, 1);
    }

    void PipelineDeffered::computeLighting()
    {
        SDL_assert(mLightComputeShader);
        
        // The shader writes the background for pixels without geometry or skybox.
        const auto &background = mCombinePass->getPtr()->cleanColor;
        mLightComputeShader->getShaderParameters().setFloatv3Parameter("ClearColor",
            kmVec3 { background.x, background.y, background.z });
        const uint32_t groupsXCount = (mCombinedTexture->getWidth() + lightComputeLocalThreadsX - 1) / lightComputeLocalThreadsX;
        const uint32_t groupsYCount = (mCombinedTexture->getHeight() + lightComputeLocalThreadsY - 1) / lightComputeLocalThreadsY;
        mLightComputeShader->dispatch(groupsXCount, groupsYCount, 1);
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
                .set(L"Value", mSSAOEnabled ? 1 : 0)
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
         * | channel01 | iwv.x             | iwv.y             | iwv.z             | geoNormal (Oct24) |
         * | channel02 | normal.x          | normal.y          | normal.z          | MatID(24)         |
         * | channel03 | albedo.rg (2x12)  | albedo.b(12)+S(12)| R(12)+M(12)       | AO                |
         * | channel04 | emission.r        | emission.g        | emission.b        | reserved          |
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
        // W всегда очищаем как NaN, для проверки валидности 
        gBufferTargetConfig.set(L"Priority", static_cast<int>(RenderPassPriority::GBuffer))
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

        mGBufferPass->setBackgroundColor(kmVec4 { 0.0f, 0.0f, 0.0f, std::numeric_limits<float>::quiet_NaN() });

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
            .set(L"Filtering", "None")
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RGBA")
            .set(L"InternalFormat", "RGBA16F")
            // SSAO всегда вычисляем в половинном разрешении, так устроен шейдер!
            .set(L"Scale", 2);

        mSSAOTexture = getMain().getResourceManager().queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_SSAO.texture", ssaoTextureConfig.write(), this);
        mSSAOTexture2 = getMain().getResourceManager().queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_SSAO2.texture", ssaoTextureConfig.write(), this);

        SDL_assert(mGBufferTexture);

        ConfigurationWriter ssaoComputeConfig;
        ssaoComputeConfig.set(L"Program", ConfigurationWriter()
                    .set(L"Name", "SSAO.program")
                    .set(L"Path", mShaderPackage + ":shaders/json/ssao.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "screenMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "GTAOOutput")
                        .set(L"Type", "imageStore")
                        .set(L"Direction", "out")
                        .set(L"TextureName", mSSAOTexture->getName()),
                    ConfigurationWriter()
                        .set(L"Name", "GBuffer")
                        .set(L"TextureName", mGBufferTexture->getName())
                        .set(L"Type", "sampler"),
                    ConfigurationWriter()
                        .set(L"Name", "CameraView")
                        .set(L"Type", "m4")
                        .set(L"Scope", "global"),
                    ConfigurationWriter()
                        .set(L"Name", "CameraProjection")
                        .set(L"Type", "m4")
                        .set(L"Scope", "global"),
                    ConfigurationWriter()
                        .set(L"Name", "Eye")
                        .set(L"Scope", "global")
                        .set(L"Type", "v3")
                });

        const auto localThreads = ShaderProgram::getOptimalWorkGroupSize(mSSAOTexture->getWidth(), mSSAOTexture->getHeight());
        ssaoComputeLocalThreadsX = localThreads[0];
        ssaoComputeLocalThreadsY = localThreads[1];
        ShaderProgram::addGlobalDefinition("LITE3D_SSAO_COMPUTE_THREADS_X", std::to_string(ssaoComputeLocalThreadsX));
        ShaderProgram::addGlobalDefinition("LITE3D_SSAO_COMPUTE_THREADS_Y", std::to_string(ssaoComputeLocalThreadsY));

        mSSAOComputeShader = getMain().getResourceManager().queryResourceFromJson<ComputeShader>(
            getName() + "_" + cameraName + "_SSAO.comp", ssaoComputeConfig.write(), this);

        ConfigurationWriter ssaoFilterConfig;
        ssaoFilterConfig.set(L"Program", ConfigurationWriter()
                .set(L"Name", "SSAOFilter.program")
                .set(L"Path", mShaderPackage + ":shaders/json/ssao_filter.json"))
            .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                ConfigurationWriter()
                    .set(L"Name", "Direction")
                    .set(L"Type", "int")
                    .set(L"Value", 1),
                ConfigurationWriter()
                    .set(L"Name", "AOMap")
                    .set(L"Type", "sampler")
                    .set(L"TextureName", mSSAOTexture->getName()),
                ConfigurationWriter()
                    .set(L"Name", "FilteredOutput")
                    .set(L"Type", "imageStore")
                    .set(L"Direction", "out")
                    .set(L"TextureName", mSSAOTexture2->getName())
            });

        mSSAOFilterShader = getMain().getResourceManager().queryResourceFromJson<ComputeShader>(
            getName() + "_" + cameraName + "_SSAOFilter.comp", ssaoFilterConfig.write(), this);
        mSSAOEnabled = true;
    }
}}
