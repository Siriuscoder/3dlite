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
    inline RenderTarget* getDiffusePass() { return mSpecularMapPass; }
    inline const std::string &getViewCubeMatrixBufferName() const { return mPositionViewCubeMatrices->getName(); }
    inline void setMainCamera(Camera *camera) { mMainCamera = camera; }
    void rebuildBuffers();

protected:

    void timerTick(lite3d_timer *timerid) override;
    void postUpdate(RenderTarget *rt) override;
    bool beginUpdate(RenderTarget *rt) override;

    void createDiffuse(const ConfigurationReader &iblConfig);
    void createDiffuseIrradiance(const ConfigurationReader &iblConfig);
    VBOResource* createMatrixBuffer(const String& bufferName);

protected:

    Main& mMain;
    String mPipelineName;
    String mShaderPackage;
    VBOResource *mPositionViewCubeMatrices = nullptr;
    VBOResource *mCenteredViewCubeMatrices = nullptr;
    RenderTarget *mSpecularMapPass = nullptr;
    RenderTarget *mIrradianceMapPass = nullptr;
    Texture *mSpecularMap = nullptr;
    Texture *mDepthCubeMap = nullptr;
    Texture *mIrradianceMap = nullptr;
    lite3d_timer *mUpdateTimer = nullptr;
    Camera *mMainCamera = nullptr;
    stl<String>::list mResourcesList;
};

}}