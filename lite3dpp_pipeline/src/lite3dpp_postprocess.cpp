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
#include <SDL_assert.h>

#include <lite3dpp_pipeline/lite3dpp_postprocess.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline_base.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    PostProcessPass::PostProcessPass(Main &main, PipelineBase &pipeline, BloomPass *bloomPass) : 
        mMain(main),
        mPipeline(pipeline),
        mBloomPass(bloomPass)
    {
        mMain.addObserver(this);
    }

    PostProcessPass::~PostProcessPass()
    {
        if (mFXAAStage)
        {
            mFXAAStage->removeObserver(this);
        }

        mMain.removeObserver(this);
    }

    void PostProcessPass::initialize(TextureImage &combinedImage)
    {
        auto postProcessConfig = mPipeline.getConfig().getObject(L"PostProcess");
        if (postProcessConfig.has(L"DynamicExposure"))
        {
            mDynamicExposureEnabled = true;
            auto dynamicExposureConfig = postProcessConfig.getObject(L"DynamicExposure");
            mExposureMax = dynamicExposureConfig.getDouble(L"ExposureMax", 1.0f);
            mExposureMin = dynamicExposureConfig.getDouble(L"ExposureMin", 1.0f);
            mExposureBase = dynamicExposureConfig.getDouble(L"ExposureBase", 1.0f);
        }

        updateSaturation(postProcessConfig.getDouble(L"Saturation", 1.0f));
        updateContrast(postProcessConfig.getDouble(L"Contrast", 1.0f));
        updateExposure(postProcessConfig.getDouble(L"Exposure", 1.0f));

        // Be sure to use GLSL 4.30
        ShaderProgram::setShaderVersion("430");

        if (postProcessConfig.getBool(L"LensFlare", false))
        {
            ShaderProgram::addGlobalDefinition("LITE3D_LENSFLARE_ENABLE", "1");
        }

        if (postProcessConfig.getBool(L"ChromaticAberration", false))
        {
            ShaderProgram::addGlobalDefinition("LITE3D_CHROMATIC_ABERRATION_ENABLE", "1");
        }

        ConfigurationWriter postProcessOutputParams;
        postProcessOutputParams.set(L"TextureType", "2D")
            .set(L"Filtering", "Linear")
            .set(L"Width", combinedImage.getWidth())
            .set(L"Height", combinedImage.getHeight())
            .set(L"Wrapping", "ClampToEdge")
            .set(L"Compression", false)
            .set(L"TextureFormat", "RGBA");

        mPostProcessOutput = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
            mPipeline.getName() + "_PostProcessOutput.texture", postProcessOutputParams.write());

        auto shaderPackage = mPipeline.getConfig().getString(L"ShaderPackage");
        stl<ConfigurationWriter>::vector postProcessVariables;
        postProcessVariables.push_back(ConfigurationWriter()
            .set(L"Name", "Combined")
            .set(L"TextureName", combinedImage.getName())
            .set(L"Type", "sampler"));
        postProcessVariables.push_back(ConfigurationWriter()
            .set(L"Name", "Exposure")
            .set(L"Value", postProcessConfig.getDouble(L"Exposure", mExposure))
            .set(L"Type", "float"));
        postProcessVariables.push_back(ConfigurationWriter()
            .set(L"Name", "Contrast")
            .set(L"Value", postProcessConfig.getDouble(L"Contrast", mContrast))
            .set(L"Type", "float"));
        postProcessVariables.push_back(ConfigurationWriter()
            .set(L"Name", "Saturation")
            .set(L"Value", postProcessConfig.getDouble(L"Saturation", mSaturation))
            .set(L"Type", "float"));
        postProcessVariables.push_back(ConfigurationWriter()
            .set(L"Type", "imageStore")
            .set(L"Name", "PostProcessOutput")
            .set(L"Direction", "output")
            .set(L"TextureName", mPostProcessOutput->getName()));

        if (mBloomPass)
        {
            postProcessVariables.emplace_back(ConfigurationWriter()
                .set(L"Name", "Bloom")
                .set(L"Type", "sampler")
                .set(L"TextureName", mBloomPass->getLastTexture().getName()));
        }

        ConfigurationWriter shaderParams;
        shaderParams.set(L"Program", ConfigurationWriter()
                .set(L"Name", "postprocess.program")
                .set(L"Path", shaderPackage + ":shaders/json/postprocess.json"))
            .set(L"Uniforms", postProcessVariables);

        mPostProcessShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>(
            mPipeline.getName() + "_PostProcess.comp", shaderParams.write(), &mPipeline);

        initializeFXAA();
    }

    void PostProcessPass::initializeFXAA()
    {
        BigTriSceneGenerator stageGenerator;
        stageGenerator.addRenderTarget(WindowRenderTarget::Name, ConfigurationWriter()
            .set(L"Priority", static_cast<int>(RenderPassStagePriority::PostProcessStage))
            .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"DepthTest", false)
            .set(L"ColorOutput", true)
            .set(L"DepthOutput", false));
            
        mFXAAStage = mMain.getResourceManager().queryResourceFromJson<Scene>(
            mPipeline.getName() + "_PostProcessStage", stageGenerator.generate().write(), &mPipeline);

        ConfigurationWriter fxaaShaderConfig;
        auto shaderPackage = mPipeline.getConfig().getString(L"ShaderPackage");

        fxaaShaderConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
            ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
                .set(L"Program", ConfigurationWriter()
                    .set(L"Name", "FXAA.program")
                    .set(L"Path", shaderPackage + ":shaders/json/fxaa.json"))
                .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"Name", "screenMatrix"),
                    ConfigurationWriter()
                        .set(L"Name", "InputImage")
                        .set(L"TextureName", mPostProcessOutput->getName())
                        .set(L"Type", "sampler"),
                    ConfigurationWriter()
                        .set(L"Name", "OutputResolution")
                        .set(L"Value", kmVec3 { static_cast<float>(mMain.window()->width()), 
                            static_cast<float>(mMain.window()->height()), 0.0f })
                        .set(L"Type", "v3")
                        .set(L"Scope", "global")
                })
        });

        // Создаем шейдер FXAA
        mFXAAStageMaterial = mMain.getResourceManager().queryResourceFromJson<Material>(
            mPipeline.getName() + "_FXAA.material", fxaaShaderConfig.write(), &mPipeline);

        // Добавляем шейдер постпроцессинга финального изображения 
        mFXAAStage->addObject("BigTri", 
            BigTriObjectGenerator(mFXAAStageMaterial->getName()).generate());

        mFXAAStage->addObserver(this);
    }

    void PostProcessPass::updateExposure(float exp)
    {
        mExposure = exp;
    }

    void PostProcessPass::updateContrast(float contrast)
    {
        mContrast = contrast;
    }

    void PostProcessPass::updateSaturation(float saturation)
    {
        mSaturation = saturation;
    }

    void PostProcessPass::frameBegin()
    {
        if (mBloomPass && mDynamicExposureEnabled)
        {
            // Обновление экспозиции каждые 10 кадров
            auto renderStats = mMain.getRenderStats();
            if ((renderStats->framesCount % 10) == 0)
            {
                auto lumaAverage = mBloomPass->getLumaAverage();
                auto exposure = mExposureBase / lumaAverage;
                exposure = std::max(mExposureMin, std::min(mExposureMax, exposure));
                updateExposure(exposure);
            }
        }
    }

    bool PostProcessPass::beginSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params)
    {
        // Update output resolution, may be dynamicaly changed 
        Material::setFloatv3GlobalParameter("OutputResolution", kmVec3 { 
            static_cast<float>(mMain.window()->width()), 
            static_cast<float>(mMain.window()->height()), 0.0f });

        SDL_assert(mPostProcessShader);

        mPostProcessShader->getShaderParameters().setFloatParameter("Exposure", mExposure);
        mPostProcessShader->getShaderParameters().setFloatParameter("Contrast", mContrast);
        mPostProcessShader->getShaderParameters().setFloatParameter("Saturation", mSaturation);

        const uint32_t groupsCountX = (mPostProcessOutput->getWidth() + 16 - 1) / 16;
        const uint32_t groupsCountY = (mPostProcessOutput->getHeight() + 16 - 1) / 16;
        mPostProcessShader->dispatch(groupsCountX, groupsCountY, 1);

        return true;
    }
}}
