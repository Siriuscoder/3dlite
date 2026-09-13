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

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline_common.h>
#include <lite3dpp_pipeline/lite3dpp_shadow_caster.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

class LITE3DPP_PIPELINE_EXPORT ShadowManager : public RenderTargetObserver, public SceneObserver, public Noncopiable
{
public:

    class LITE3DPP_PIPELINE_EXPORT VisibilityHintNode : public SceneNodeObserver, public Noncopiable
    {
    public:
    
        using AffectingShadowCasters = stl<ShadowCaster*>::unordered_set;

        VisibilityHintNode(SceneNodeBase *node, bool recursive);
        ~VisibilityHintNode();

        void resetVision();
        void setVisibleFrom(ShadowCaster* sc);
        void setInvisibleFrom(ShadowCaster* sc);
        bool isRecursive();

    private:

        void updatePosition(SceneNodeBase *node) override;
        void updateRotation(SceneNodeBase *node) override;
        void updateScale(SceneNodeBase *node) override;
        void updateSkeletonPose(SceneNodeBase *node) override;
        void invalidate();

        SceneNodeBase *mNode = nullptr;
        AffectingShadowCasters mAffectingShadowCasters;
        bool mRecursive;
    };

public:

    using IndexVector = stl<int32_t>::vector;
    using ShadowCasters = stl<LightSceneNode *, std::unique_ptr<ShadowCaster>>::multimap;
    using VisibilityHints = stl<SceneNodeBase *, std::unique_ptr<VisibilityHintNode>>::unordered_map;

    ShadowManager(Main& main, PipelineBase &pipeline);
    ~ShadowManager();

    void initialize();
    stl<ShadowCaster*>::vector registerEmitter(LightSceneNode* emitter);
    void unregisterEmitter(LightSceneNode* emitter);
    VisibilityHintNode* registerHintNode(SceneNodeBase *node, bool recursive = false);
    void unregisterHintNode(SceneNodeBase *node);
    void clear();

    inline RenderTarget& getShadowPass()
    {
        return *mShadowPass;
    }

    inline Texture* getShadowMapTexture()
    {
        return mShadowMap;
    }

    inline VBOResource* getShadowMatrixBuffer()
    {
        return mShadowMatrixBuffer;
    }

    inline VBOResource* getShadowIndexBuffer()
    {
        return mShadowIndexBuffer;
    }

    inline uint32_t getShadowsCacheMaxCount() const 
    { 
        return mSpotShadowCacheMaxCount + (mOmniShadowCacheMaxCount * 6) + mCascadeShadowCacheMaxCount;
    }


protected:

    bool beginUpdate(RenderTarget *rt) override;
    void postUpdate(RenderTarget *rt) override;
    bool beginSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params) override;
    void endSceneRender(Scene *scene, Camera *camera, const lite3d_scene_render_params *params) override;

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool customFrustumCheck(Scene *scene, SceneNodeBase *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera, const lite3d_scene_render_params *params) override;

    void createShadowRenderTarget();
    void createAuxiliaryBuffers();
    void setupLimits();

private:

    Main& mMain;
    PipelineBase &mPipeline;
    uint32_t mSpotShadowCacheMaxCount = 0;
    uint32_t mOmniShadowCacheMaxCount = 0;
    uint32_t mCascadeShadowCacheMaxCount = 0;
    uint32_t mMaxShadowsRebuildCount = 0;
    uint32_t mExtent = 0;
    RenderTarget* mShadowPass = nullptr;
    Texture* mShadowMap = nullptr;
    VBOResource* mShadowMatrixBuffer = nullptr;
    VBOResource* mShadowIndexBuffer = nullptr;
    IndexVector mHostShadowIndexes;
    ShadowCasters mShadowCasters;
    stl<ShadowCaster*>::vector mShadowCastersCachePlaceHolders;
    VisibilityHints mVisibilityHintNodes;
    Scene *mCleanStage = nullptr;
    bool mCascadeShadowIsReserved = false;
};

}}
