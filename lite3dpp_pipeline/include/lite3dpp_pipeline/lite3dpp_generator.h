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

#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    enum class RenderPassPriority
    {
        ShadowMap = 1,
        MainDepth = 2
    };

    enum class RenderPassStagePriority
    {
        ShadowCleanStage = 0,
        ShadowBuildStage = 1,
        DepthBuildStage = 0
    };

    enum class TexturePassTypes
    {
        Depth = 1,
        RenderPass = 2,
        Shadow = 3
    };

    class SceneGenerator 
    {
    public:

        SceneGenerator(const String& sceneName) : 
            mName(sceneName)
        {}

        ConfigurationWriter generate();
        virtual ConfigurationWriter& generateFromExisting(ConfigurationWriter& sceneConfig);
        void addCamera(const String& cameraName, ConfigurationWriter& conf);
        void addRenderTarget(const String& cameraName, const String& renderTargetName, ConfigurationWriter& conf);

        inline String &getName()
        {
            return mName;
        }

    protected:

        String mName;
        stl<String, ConfigurationWriter>::map mCameras;
        stl<String, stl<ConfigurationWriter>::vector>::map mRenderTargets;
    };

    class BigTriSceneGenerator : public SceneGenerator
    {
    public:

        BigTriSceneGenerator(const String& sceneName) : 
            SceneGenerator(sceneName)
        {}

        ConfigurationWriter& generateFromExisting(ConfigurationWriter& sceneConfig) override;
    };

    class BigTriObjectGenerator
    {
    public:
    
        BigTriObjectGenerator(const String& materialName) : 
            mMaterialName(materialName)
        {}

        ConfigurationReader generate();

    private:

        String mMaterialName;
    };
}}
