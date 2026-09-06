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

    ShadowCaster::ShadowCaster(EmitterType emitterType, Main& main, LightSceneNode* node) : 
        mLightNode(node),
        mEmitterType(emitterType),
        mMain(main)
    {
        SDL_assert(node);
    }

    ShadowCaster::~ShadowCaster()
    {
        for (auto camera : mCameras)
        {
            mMain.removeCamera(camera->getName());
        }
    }

    void ShadowCaster::recalcMatrices(stl<kmMat4>::vector &matrices)
    {
        matrices.clear();
        for (auto camera : mCameras)
        {
            camera->setDirection(mLightNode->getLight()->getWorldDirection());
            camera->setPosition(mLightNode->getLight()->getWorldPosition());
            camera->recalcFrustum();
            matrices.push_back(camera->refreshProjViewMatrix());
        }
    }

    bool ShadowCaster::intersectFrustum(const lite3d_bounding_vol &aabb)
    {
        for (auto camera : mCameras)
        {
            if (camera->intersectFrustum(aabb))
                return true;
        }

        return false;
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

    ShadowCasterSpot::ShadowCasterSpot(Main &main, LightSceneNode *emitter) : 
        ShadowCaster(ShadowCaster::EmitterType::SpotShadow, main, emitter)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypeDiskArea || 
            mLightNode->getLight()->getType() == LightSourceFlags::TypeRectArea || 
            mLightNode->getLight()->getType() == LightSourceFlags::TypeSpot);

        const auto &clip = mLightNode->getLight()->getShadowClip();
        auto clipFar = clip.farClipPlane > FLT_EPSILON ? clip.farClipPlane : 
            mLightNode->getLight()->getInfluenceDistance();
                    
        auto camera = main.addCamera(mLightNode->getName() + "_spot_shadow");
        camera->setupPerspective(clip.nearClipPlane, clipFar, 
            kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), 1.0);
        mCameras.push_back(camera);
    }

    ShadowCasterOmniDirectional::ShadowCasterOmniDirectional(Main &main, LightSceneNode *emitter) : 
        ShadowCaster(ShadowCaster::EmitterType::OmniDirectionalShadow, main, emitter)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypePoint);

        const auto &clip = mLightNode->getLight()->getShadowClip();
        auto clipFar = clip.farClipPlane > FLT_EPSILON ? clip.farClipPlane : 
            mLightNode->getLight()->getInfluenceDistance();

        auto camera = main.addCamera(mLightNode->getName() + "_omni_shadow");
        camera->setupPerspective(clip.nearClipPlane, clipFar, 90.0, 1.0);
        mCameras.push_back(camera);
    }

    void ShadowCasterOmniDirectional::recalcMatrices(stl<kmMat4>::vector &matrices)
    {
        mCameras[0]->setDirection(mLightNode->getLight()->getWorldDirection());
        mCameras[0]->setPosition(mLightNode->getLight()->getWorldPosition());
        mCameras[0]->computeCubeProjView(matrices);
    }

    bool ShadowCasterOmniDirectional::intersectFrustum(const lite3d_bounding_vol &aabb)
    {
        auto distance = mCameras[0]->getDistance(aabb.sphereCenter);
        return distance <= (aabb.radius + mLightNode->getLight()->getInfluenceDistance());
    }

    ShadowCasterCascade::ShadowCasterCascade(Main &main, LightSceneNode *emitter, uint32_t cascadeMaxCount) : 
        ShadowCaster(ShadowCaster::EmitterType::CascadeShadow, main, emitter)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypePoint);

        const auto &clip = mLightNode->getLight()->getShadowClip();
        mCameras.resize(cascadeMaxCount);

        for (auto it = mCameras.rbegin(); it != mCameras.rend(); ++it)
        {
            auto cameraName = mLightNode->getName() + "_cascade_shadow_" + std::to_string(cascadeMaxCount);
            *it = main.addCamera(cameraName);

            (*it)->setupOrtho(clip.nearClipPlane, clip.farClipPlane, 
                clip.leftClipPlane / cascadeMaxCount, 
                clip.rightClipPlane / cascadeMaxCount, 
                clip.bottomClipPlane / cascadeMaxCount,
                clip.topClipPlane / cascadeMaxCount);

            cascadeMaxCount--;
        }
    }

}}
