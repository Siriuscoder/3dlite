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
#pragma once 

#include <lite3dpp/lite3dpp_compute_shader.h>
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

class LITE3DPP_PIPELINE_EXPORT PostProcessPass : public SceneObserver, public LifecycleObserver, public Noncopiable
{
public:

    PostProcessPass(Main &main, PipelineBase &pipeline, BloomPass *bloomPass);
    virtual ~PostProcessPass();

    void initialize(TextureImage &combinedImage);
    void updateExposure(float exp);
    void updateContrast(float contrast);
    void updateSaturation(float saturation);

protected:

    void initializeFXAA();
    void frameBegin() override;
    bool beginSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params) override;

protected:

    Main &mMain;
    PipelineBase &mPipeline;
    BloomPass *mBloomPass;
    float mExposureMax = 1.0;
    float mExposureMin = 1.0;
    float mExposureBase = 1.0;
    float mExposure = 1.0;
    float mContrast = 1.0;
    float mSaturation = 1.0;
    bool mDynamicExposureEnabled = false;
    ComputeShader *mPostProcessShader = nullptr;
    TextureImage *mPostProcessOutput;
    Scene *mFXAAStage = nullptr;
    Material *mFXAAStageMaterial = nullptr;
};

}}
