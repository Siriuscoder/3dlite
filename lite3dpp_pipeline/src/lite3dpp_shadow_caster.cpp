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
#include <lite3dpp_pipeline/lite3dpp_shadow_caster.h>

#include <algorithm>
#include <cmath>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    uint32_t ShadowCaster::gCameraCounter = 0;

    ShadowCaster::ShadowCaster(EmitterType emitterType, Main& main, LightSceneNode* emitter) : 
        mLightNode(emitter),
        mEmitterType(emitterType),
        mMain(main)
    {
        SDL_assert(emitter);

        // Если какой либо из узлов сцены поменяет свое положение тень нужно перерисовать
        SceneNodeBase *node = emitter;
        while (node)
        {
            node->addObserver(this);
            node = node->getParent();
        }
    }

    ShadowCaster::~ShadowCaster()
    {
        mMain.removeCamera(mCamera->getName());
        
        SceneNodeBase *node = mLightNode;
        while (node)
        {
            node->removeObserver(this);
            node = node->getParent();
        }
    }

    kmMat4 ShadowCaster::recalcMatrix()
    {
        mCamera->setDirection(mLightNode->getLight()->getWorldDirection());
        mCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        mCamera->recalcFrustum();
        return mCamera->refreshProjViewMatrix();
    }

    bool ShadowCaster::intersectFrustum(const lite3d_bounding_vol &aabb)
    {
        return mCamera->intersectFrustum(aabb);
    }

    void ShadowCaster::updatePosition(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowCaster::updateRotation(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowCaster::updateScale(SceneNodeBase *node)
    {
        invalidate();
    }

    void ShadowCaster::updateSkeletonPose(SceneNodeBase *node)
    {
        invalidate();
    }

    int32_t ShadowCaster::getCacheIndex() const
    {
        return mLightNode->getLight()->getShadowIndex();
    }

    void ShadowCaster::setCacheIndex(int32_t index)
    {
        mLightNode->getLight()->setShadowIndex(index);
    }

    ShadowCasterSpot::ShadowCasterSpot(Main &main, LightSceneNode *emitter) : 
        ShadowCaster(ShadowCaster::EmitterType::SpotShadow, main, emitter)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypeDiskArea || 
            mLightNode->getLight()->getType() == LightSourceFlags::TypeRectArea || 
            mLightNode->getLight()->getType() == LightSourceFlags::TypeSpot);

        auto clipFar = mLightNode->getLight()->getClipFar() > FLT_EPSILON ? mLightNode->getLight()->getClipFar() : 
            mLightNode->getLight()->getInfluenceDistance();
                    
        mCamera = main.addCamera(mLightNode->getName() + "_spot_shadow_" + std::to_string(++gCameraCounter));
        mCamera->setupPerspective(mLightNode->getLight()->getClipNear(), clipFar, 
            kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), 1.0);
    }

    ShadowCasterOmniDirectional::ShadowCasterOmniDirectional(Main &main, LightSceneNode *emitter, uint32_t faceNum) : 
        ShadowCaster(ShadowCaster::EmitterType::OmniDirectionalShadow, main, emitter),
        mFaceNum(faceNum)
    {
        const kmVec3 faceDirections[] = {
            KM_VEC3_POS_X,
            KM_VEC3_NEG_X,
            KM_VEC3_POS_Y,
            KM_VEC3_NEG_Y,
            KM_VEC3_POS_Z,
            KM_VEC3_NEG_Z
        };

        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypePoint);
        SDL_assert(faceNum >= 0 && faceNum <= 6);

        auto clipFar = mLightNode->getLight()->getClipFar() > FLT_EPSILON ? mLightNode->getLight()->getClipFar() : 
            mLightNode->getLight()->getInfluenceDistance();

        mCamera = main.addCamera(mLightNode->getName() + "_omni_shadow_face_" + std::to_string(faceNum) + "_" + 
            std::to_string(++gCameraCounter));
        mCamera->setupPerspective(mLightNode->getLight()->getClipNear(), clipFar, 90.0, 1.0);
        mCamera->setDirection(faceDirections[faceNum]);
    }

    int32_t ShadowCasterOmniDirectional::getCacheIndex() const
    {
        if (mLightNode->getLight()->getShadowIndex() >= 0)
        {
            return mLightNode->getLight()->getShadowIndex() + mFaceNum;
        }

        return -1;
    }

    void ShadowCasterOmniDirectional::setCacheIndex(int32_t index)
    {
        if (mFaceNum == 0)
        {
            mLightNode->getLight()->setShadowIndex(index);
        }
    }

    kmMat4 ShadowCasterOmniDirectional::recalcMatrix()
    {
        mCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        mCamera->recalcFrustum();
        return mCamera->refreshProjViewMatrix();
    }

    ShadowCasterCascade::ShadowCasterCascade(Main &main, LightSceneNode *emitter, uint32_t cascadeNum) : 
        ShadowCaster(ShadowCaster::EmitterType::CascadeShadow, main, emitter),
        mCascadeNum(cascadeNum)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypePoint);
    }

    int32_t ShadowCasterCascade::getCacheIndex() const
    {
        if (mLightNode->getLight()->getShadowIndex() >= 0)
        {
            return mLightNode->getLight()->getShadowIndex() + mCascadeNum;
        }

        return -1;
    }

    void ShadowCasterCascade::setCacheIndex(int32_t index)
    {
        if (mCascadeNum == 0)
        {
            mLightNode->getLight()->setShadowIndex(index);
        }
    }
}}
