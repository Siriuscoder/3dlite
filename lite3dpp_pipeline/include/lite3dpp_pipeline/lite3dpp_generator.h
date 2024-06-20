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
#include <lite3dpp_pipeline/lite3dpp_pipeline_common.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    enum class RenderPassPriority
    {
        MainDepth = 1,
        ShadowMap = 2,
        GBuffer = 3,
        SSAO = 4,
        Combine = 5,
        BLOOM = 6,
        PostProcess = 7
    };

    enum class RenderPassStagePriority
    {
        ShadowCleanStage = 0,
        ShadowBuildStage = 1,
        DepthBuildStage = 0,
        BloomBuildStage = 0,
        LightComputeStage = 0,
        GBufferBuildStage = 0,
        ForwardStage = 0,
        BlendDecalStage = 2,
        SSAOBuildStage = 0,
        PostProcessStage = 0,
        SkyBoxStage = 1
    };

    enum class TexturePassTypes
    {
        Depth = 1,
        RenderPass = 2,
        Shadow = 3
    };

    class LITE3DPP_PIPELINE_EXPORT SceneGenerator 
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

    class LITE3DPP_PIPELINE_EXPORT BigTriSceneGenerator : public SceneGenerator
    {
    public:

        BigTriSceneGenerator() : 
            SceneGenerator("")
        {}

        ConfigurationWriter& generateFromExisting(ConfigurationWriter& sceneConfig) override;
        void addRenderTarget(const String& renderTargetName, ConfigurationWriter& conf);
    };

    class LITE3DPP_PIPELINE_EXPORT SimpleObjectGenerator
    {
    public:
    
        SimpleObjectGenerator(const String& materialName, const String& meshName) : 
            mMaterialName(materialName),
            mMeshName(meshName)
        {}

        ConfigurationReader generate();

    private:

        String mMaterialName;
        String mMeshName;
    };

    class LITE3DPP_PIPELINE_EXPORT BigTriObjectGenerator : public SimpleObjectGenerator
    {
    public:
    
        BigTriObjectGenerator(const String& materialName) : 
            SimpleObjectGenerator(materialName, "BigTriangle.mesh")
        {}
    };

    class LITE3DPP_PIPELINE_EXPORT SkyBoxObjectGenerator : public SimpleObjectGenerator
    {
    public:
    
        SkyBoxObjectGenerator(const String& materialName) : 
            SimpleObjectGenerator(materialName, "SkyBox.mesh")
        {}
    };
}}
