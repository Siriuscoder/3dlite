/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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

class LITE3DPP_PIPELINE_EXPORT ShadowManager : public RenderTargetObserver, public SceneObserver, public Noncopiable
{
public:

    using IndexVector = stl<int32_t>::vector;

public:

    class LITE3DPP_PIPELINE_EXPORT ShadowCaster : public SceneNodeObserver
    {
    public:
        
        ShadowCaster(Main& main, const String& name, LightSceneNode* node, 
            const lite3d_camera::projectionParamsStruct &params);

        kmMat4 getMatrix();

        inline LightSceneNode* getNode()
        {
            return mLightNode;
        }

        inline Camera* getCamera()
        {
            return mShadowCamera;
        }

        inline bool invalidated() const
        {
            return mInvalidated;
        }

        inline void validate()
        {
            mInvalidated = false;
        }

        inline void invalidate()
        {
            mInvalidated = true;
        }

    private:

        void updatePosition(SceneNodeBase *node) override;
        void updateRotation(SceneNodeBase *node) override;
        void updateScale(SceneNodeBase *node) override;

    private:

        LightSceneNode* mLightNode = nullptr;
        Camera* mShadowCamera = nullptr;
        bool mInvalidated = true;
    };

    class LITE3DPP_PIPELINE_EXPORT VisibilityHintNode : public SceneNodeObserver
    {
    public:
    
        using ShadowCasters = stl<ShadowCaster *>::vector;

        VisibilityHintNode(SceneNodeBase *node);

        void resetVision();
        void setVisibleFrom(ShadowCaster* sc);

    private:

        void updatePosition(SceneNodeBase *node) override;
        void updateRotation(SceneNodeBase *node) override;
        void updateScale(SceneNodeBase *node) override;
        void invalidate();

        SceneNodeBase *mNode = nullptr;
        ShadowCasters mVisibility;
    };

    ShadowManager(Main& main, const String& pipelineName, const ConfigurationReader& pipelineConf);
    ~ShadowManager();

    void initialize(const String& pipelineName, const String& shaderPackage);
    ShadowCaster* newShadowCaster(LightSceneNode* node);
    VisibilityHintNode* registerHintNode(SceneNodeBase *node);
    VisibilityHintNode* registerHintNodeRecursive(SceneNodeBase *node);
    void unregisterHintNode(SceneNodeBase *node);
    void unregisterHintNodeRecursive(SceneNodeBase *node);

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

    inline uint32_t getShadowsCastersMaxCount() const 
    { 
        return mShadowsCastersMaxCount;
    }


protected:

    bool beginUpdate(RenderTarget *rt) override;
    void postUpdate(RenderTarget *rt) override;
    bool beginSceneRender(Scene *scene, Camera *camera) override;
    void endSceneRender(Scene *scene, Camera *camera) override;

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool customVisibilityCheck(Scene *scene, SceneNodeBase *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera) override;

    void createShadowRenderTarget(const String& pipelineName);
    void createAuxiliaryBuffers(const String& pipelineName);
    void calculateLimits();

private:

    Main& mMain;
    uint32_t mShadowsCastersMaxCount;
    uint32_t mWidth, mHeight;
    RenderTarget* mShadowPass = nullptr;
    Texture* mShadowMap = nullptr;
    VBOResource* mShadowMatrixBuffer = nullptr;
    VBOResource* mShadowIndexBuffer = nullptr;
    IndexVector mHostShadowIndexes;
    stl<std::unique_ptr<ShadowCaster>>::vector mShadowCasters;
    stl<SceneNodeBase *, std::shared_ptr<VisibilityHintNode>>::unordered_map mVisibilityHintNodes;
    lite3d_camera::projectionParamsStruct mProjection = {};
    Scene *mCleanStage = nullptr;
    Material *mCleanStageMaterial = nullptr;
};

}}