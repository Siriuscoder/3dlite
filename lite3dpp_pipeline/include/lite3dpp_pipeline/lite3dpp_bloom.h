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

class LITE3DPP_PIPELINE_EXPORT BloomEffect : public SceneObserver, public Noncopiable
{
public:

    BloomEffect(Main& main, const String &pipelineName, const String &cameraName, const ConfigurationReader &pipelineConfig);
    ~BloomEffect();

    TextureRenderTarget &getRenderTarget();
    TextureImage &getLastTexture();
    TextureImage &getMiddleTexture();
    kmVec3 getLumaAverage() const;

private:

    bool beginDrawBatch(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material) override;
    bool beginSceneRender(Scene *scene, Camera *camera) override;

    void init();
    void initTextureChain();
    void initBoomScene();

private:

    Main& mMain;
    String mPipelineName;
    String mCameraName;
    String mShaderPackage;
    int32_t mMinWidth = 1;
    Scene* mBloomRernderer = nullptr;
    TextureRenderTarget* mBloomRT = nullptr;
    stl<TextureImage*>::vector mTextureChain;
    stl<Material*>::vector mMaterialChain;
    TextureImage *mMiddleTexture = nullptr;
    int mChainState = 0;
    float mBloomRadius = 0.005;
    mutable PixelsData mBloomPixels;
};

}}
