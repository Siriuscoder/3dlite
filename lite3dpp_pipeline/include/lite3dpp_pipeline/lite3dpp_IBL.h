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

    IBLDiffuseIrradiance(Main& main);
    virtual ~IBLDiffuseIrradiance();

    void initialize(const String& pipelineName, const ConfigurationReader &pipelineConfig);
    inline RenderTarget* getDiffusePass() { return mSurroundDiffusePass; }
    inline void setMainCamera(Camera *camera) { mMainCamera = camera; }

protected:

    void timerTick(lite3d_timer *timerid) override;
    void postUpdate(RenderTarget *rt) override;
    bool beginUpdate(RenderTarget *rt) override;

    void createDiffuse(const String& pipelineName, const ConfigurationReader &iblConfig);
    void createDiffuseIrradiance(const String& pipelineName, const ConfigurationReader &iblConfig);
    VBOResource* createMatrixBuffer(const String& pipelineName, const String& bufferName);

protected:

    Main& mMain;
    VBOResource *mPositionViewCubeMatrices = nullptr;
    VBOResource *mCenteredViewCubeMatrices = nullptr;
    RenderTarget *mSurroundDiffusePass = nullptr;
    RenderTarget *mSurroundIrradiancePass = nullptr;
    Texture *mSurroundDiffuseTexture = nullptr;
    Texture *mSurroundDepthTexture = nullptr;
    Texture *mSurroundIrradianceTexture = nullptr;
    lite3d_timer *mUpdateTimer = nullptr;
    Camera *mMainCamera = nullptr;
    stl<String>::list mResourcesList;
};

}}