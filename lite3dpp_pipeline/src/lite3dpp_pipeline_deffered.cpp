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
#include <lite3dpp_pipeline/lite3dpp_pipeline_deffered.h>

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
        constructSSBOPass(pipelineConfig, cameraName);
        constructCombinedPass(pipelineConfig, cameraName, sceneGenerator);
        constructBloomPass(pipelineConfig, cameraName);
        constructPostProcessPass(pipelineConfig, cameraName, sceneGenerator);
    }

    void PipelineDeffered::unloadImpl()
    {
        PipelineBase::unloadImpl();

        if (mGBufferPass)
        {
            getMain().getResourceManager()->releaseResource(mGBufferPass->getName());
        }

        if (mCombinePass)
        {
            getMain().getResourceManager()->releaseResource(mCombinePass->getName());
        }

        if (mGBufferTexture)
        {
            getMain().getResourceManager()->releaseResource(mGBufferTexture->getName());
        }

        if (mCombinedTexture)
        {
            getMain().getResourceManager()->releaseResource(mCombinedTexture->getName());
        }
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

        mCombinedTexture = getMain().getResourceManager()->queryResourceFromJson<TextureImage>(
            getName() + "_" + cameraName + "_combined.texture", combinedTextureConfig.write());

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

        sceneGenerator.addRenderTarget(cameraName, mGBufferPass->getName(), ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::BlendDecalStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", true)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", true)
            .set(L"RenderBlend", true)
            .set(L"RenderOpaque", false)
            .set(L"RenderInstancing", pipelineConfig.getBool(L"Instancing", true)));
    }

    void PipelineDeffered::constructSSBOPass(const ConfigurationReader &pipelineConfig, const String &cameraName)
    {

    }

    void PipelineDeffered::constructPostProcessPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
        SceneGenerator &sceneGenerator)
    {

    }
}}
