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

    Scene &PipelineBase::getSkyBoxScene()
    {
        SDL_assert(mSkyBox);
        return *mSkyBox;
    }

    ShadowManager& PipelineBase::getShadowManager()
    {
        SDL_assert(mShadowManager);
        return *mShadowManager.get();
    }

    void PipelineBase::loadFromConfigImpl(const ConfigurationReader &pipelineConfig)
    {
        mShaderPackage = pipelineConfig.getString(L"ShaderPackage");
        for (const auto &cameraConfig : pipelineConfig.getObjects(L"Cameras"))
        {
            if (cameraConfig.has(L"Main"))
            {
                mMainCamera = cameraConfig.getString(L"Main");
            }
        }

        if (mMainCamera.empty())
        {
            LITE3D_THROW("Pipeline " << getName() << ": Main camera is not set");
        }

        auto lightingTechnique = pipelineConfig.getString(L"LightingTechnique");
        auto mainScenePath = pipelineConfig.getString(L"MainScenePath");
        auto sceneJsonData = getMain().getResourceManager()->loadFileToMemory(mainScenePath);
        ConfigurationWriter sceneGeneratedConfig(static_cast<const char *>(sceneJsonData->fileBuff), sceneJsonData->fileSize);
        ConfigurationReader sceneConfig(static_cast<const char *>(sceneJsonData->fileBuff), sceneJsonData->fileSize);

        if (!lightingTechnique.empty())
        {
            sceneGeneratedConfig.set(L"LightingTechnique", lightingTechnique);
        }

        stl<ConfigurationWriter>::vector camerasPipelinesConfigs;
        for (const auto &cameraConfig : sceneConfig.getObjects(L"Cameras"))
        {
            if (mMainCamera != cameraConfig.getString(L"Name"))
                continue;

            ConfigurationWriter cameraPipelineConfig;
            cameraPipelineConfig.set(L"Name", cameraConfig.getString(L"Name"))
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

            constructCameraPipeline(pipelineConfig, cameraPipelineConfig);
            camerasPipelinesConfigs.emplace_back(cameraPipelineConfig);
        }

        sceneGeneratedConfig.set(L"Cameras", camerasPipelinesConfigs);
        
    }

    void PipelineBase::unloadImpl()
    {

    }

    void PipelineBase::constructCameraPipeline(const ConfigurationReader &pipelineConfig, 
        ConfigurationWriter &cameraPipelineConfig)
    {

    }
}}