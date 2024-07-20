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

#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

class LITE3DPP_PIPELINE_EXPORT IBLDiffuseIrradiance : public RenderTargetObserver, public LifecycleObserver, public Noncopiable
{
public:

    IBLDiffuseIrradiance(Main& main, const String& pipelineName, const String& shaderPackage);
    virtual ~IBLDiffuseIrradiance();

    void initialize(const ConfigurationReader &pipelineConfig);
    inline RenderTarget* getDiffusePass() { return mEnvironmentProbePass; }
    inline const std::string &getViewCubeMatrixBufferName() const { return mViewCubeMatricesGPUBuffer->getName(); }
    inline void setMainCamera(Camera *camera) { mMainCamera = camera; }
    inline Texture *getEnvProbeTexture() { return mEnvironmentProbe; }
    inline Texture *getIrradianceProbeTexture() { return mIrradianceProbe; }
    void rebuildEnvironmentProbe();
    void rebuildIrradianceProbe();

protected:

    void postUpdate(RenderTarget *rt) override;
    bool beginUpdate(RenderTarget *rt) override;

    void createDiffuse(const ConfigurationReader &iblConfig);
    void createDiffuseIrradiance(const ConfigurationReader &iblConfig);
    VBOResource* createMatrixBuffer(const String& bufferName);

protected:

    Main& mMain;
    String mPipelineName;
    String mShaderPackage;
    VBOResource *mViewCubeMatricesGPUBuffer = nullptr;
    RenderTarget *mEnvironmentProbePass = nullptr;
    RenderTarget *mIrradianceProbePass = nullptr;
    Texture *mEnvironmentProbe = nullptr;
    Texture *mDepthCubeMap = nullptr;
    Texture *mIrradianceProbe = nullptr;
    Camera *mMainCamera = nullptr;
    Camera *mEnvConvolutionCamera = nullptr;
    std::optional<kmVec3> mEnvProbeFixedPos;
    stl<kmMat4>::vector mMatricesHostBuffer;
    stl<String>::list mResourcesList;
};

}}