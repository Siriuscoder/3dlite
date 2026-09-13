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

namespace lite3dpp {
namespace lite3dpp_pipeline {

class LITE3DPP_PIPELINE_EXPORT ShadowCaster : public SceneNodeObserver, public Noncopiable
{
public:
    enum class EmitterType
    {
        SpotShadow,
        OmniDirectionalShadow,
        CascadeShadow
    };

    ShadowCaster(EmitterType emitterType, Main &main, LightSceneNode *emitter);
    virtual ~ShadowCaster();

    virtual kmMat4 recalcMatrix();
    virtual bool intersectFrustum(const lite3d_bounding_vol &aabb);
    virtual int32_t getCacheIndex() const;
    virtual void setCacheIndex(int32_t index); 

    inline LightSceneNode *getNode()
    {
        return mLightNode;
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

    inline bool dynamicShadow() const
    {
        return (mLightNode->getLight()->getFlags() & LightSourceFlags::ShadowDynamic) == LightSourceFlags::ShadowDynamic;
    }

    inline EmitterType getEmitterType() const 
    {
        return mEmitterType;
    }

    inline int32_t cached() const
    {
        return mLightNode->getLight()->getShadowIndex() >= 0;
    }

protected:

    void updatePosition(SceneNodeBase *node) override;
    void updateRotation(SceneNodeBase *node) override;
    void updateScale(SceneNodeBase *node) override;
    void updateSkeletonPose(SceneNodeBase *node) override;

protected:

    LightSceneNode *mLightNode;
    Camera *mCamera;
    bool mInvalidated = true;
    EmitterType mEmitterType;
    Main &mMain;
    static uint32_t gCameraCounter;
};

class LITE3DPP_PIPELINE_EXPORT ShadowCasterSpot : public ShadowCaster
{
public:

    ShadowCasterSpot(Main &main, LightSceneNode *emitter);
};

class LITE3DPP_PIPELINE_EXPORT ShadowCasterOmniDirectional : public ShadowCaster
{
public:

    ShadowCasterOmniDirectional(Main &main, LightSceneNode *emitter, uint32_t faceNum);
    kmMat4 recalcMatrix() override;
    int32_t getCacheIndex() const override;
    void setCacheIndex(int32_t index) override;

private:

    uint32_t mFaceNum;
};

class LITE3DPP_PIPELINE_EXPORT ShadowCasterCascade : public ShadowCaster
{
public:

    ShadowCasterCascade(Main &main, LightSceneNode *emitter, uint32_t cascadeNum);
    int32_t getCacheIndex() const override;
    void setCacheIndex(int32_t index) override;
    
private:

    uint32_t mCascadeNum;
};

}}
