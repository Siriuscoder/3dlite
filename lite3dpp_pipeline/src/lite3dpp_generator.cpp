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
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    ConfigurationWriter SceneGenerator::generate()
    {
        ConfigurationWriter sceneConfig;
        return generateFromExisting(sceneConfig);
    }

    ConfigurationWriter& SceneGenerator::generateFromExisting(ConfigurationWriter& sceneConfig)
    {
        stl<ConfigurationWriter>::vector camerasList;
        for (auto& cameraConfig : mCameras)
        {
            auto it = mRenderTargets.find(cameraConfig.first);
            if (it != mRenderTargets.end())
            {
                cameraConfig.second.set(L"RenderTargets", it->second);
            }

            camerasList.emplace_back(cameraConfig.second);
        }

        sceneConfig.set(L"Cameras", camerasList);

        return sceneConfig;
    }

    void SceneGenerator::addCamera(const String& cameraName, ConfigurationWriter& conf)
    {
        auto cameraFullName = mName + "_" + cameraName;
        conf.set(L"Name", cameraFullName);
        mCameras.emplace(cameraFullName, conf);
    }

    void SceneGenerator::addRenderTarget(const String& cameraName, const String& renderTargetName, ConfigurationWriter& conf)
    {
        auto cameraFullName = mName + "_" + cameraName;
        conf.set(L"Name", renderTargetName);
        mRenderTargets[cameraFullName].emplace_back(conf);
    }

    ConfigurationWriter& BigTriSceneGenerator::generateFromExisting(ConfigurationWriter& sceneConfig)
    {
        addCamera("FullScreenView", ConfigurationWriter()
            .set(L"Position", kmVec3 { 0.0f, 0.0f, 0.5f })
            .set(L"LookAt", KM_VEC3_ZERO)
            .set(L"Ortho", ConfigurationWriter()
                .set(L"Near", 0.0f)
                .set(L"Far", 1.0f)
                .set(L"Left", 0.0f)
                .set(L"Right", 1.0f)
                .set(L"Bottom", 0.0f)
                .set(L"Top", 1.0f)));

        return SceneGenerator::generateFromExisting(sceneConfig);
    }

    ConfigurationReader BigTriObjectGenerator::generate()
    {
        auto data = ConfigurationWriter().set(L"Root", ConfigurationWriter()
            .set(L"Name", "BigTriangle.root")
            .set(L"FrustumTest", false)
            .set(L"Mesh", ConfigurationWriter()
                .set(L"Name", "BigTriangle.mesh")
                .set(L"MaterialMapping", stl<ConfigurationWriter>::vector {
                    ConfigurationWriter()
                        .set(L"MaterialIndex", 0)
                        .set(L"Material", ConfigurationWriter()
                            .set(L"Name", mMaterialName)
                        )
                })
            )
        ).write();

        return ConfigurationReader(data.data(), data.size());
    }
}}
