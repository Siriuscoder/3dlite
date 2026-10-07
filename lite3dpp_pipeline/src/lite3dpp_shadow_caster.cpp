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
        if (mCamera)
        {
            mMain.removeCamera(mCamera->getName());
        }

        SceneNodeBase *node = mLightNode;
        while (node)
        {
            node->removeObserver(this);
            node = node->getParent();
        }
    }

    bool ShadowCaster::recalcMatrix(kmMat4 &matrix)
    {
        mCamera->setDirection(mLightNode->getLight()->getWorldDirection());
        mCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        matrix = mCamera->refreshProjViewMatrix();
        return true;
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

        auto clipFar = mLightNode->getLight()->getShadowClipFar() > FLT_EPSILON ? mLightNode->getLight()->getShadowClipFar() : 
            mLightNode->getLight()->getInfluenceDistance();
                    
        mCamera = main.addCamera(mLightNode->getName() + "_spot_shadow_" + std::to_string(++gCameraCounter));
        mCamera->setupPerspective(mLightNode->getLight()->getShadowClipNear(), clipFar, 
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

        auto clipFar = mLightNode->getLight()->getShadowClipFar() > FLT_EPSILON ? mLightNode->getLight()->getShadowClipFar() : 
            mLightNode->getLight()->getInfluenceDistance();

        mCamera = main.addCamera(mLightNode->getName() + "_omni_shadow_face_" + std::to_string(faceNum) + "_" + 
            std::to_string(++gCameraCounter));
        mCamera->setupPerspective(mLightNode->getLight()->getShadowClipNear(), clipFar, 90.0, 1.0);
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

    bool ShadowCasterOmniDirectional::recalcMatrix(kmMat4 &matrix)
    {
        mCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        matrix = mCamera->refreshProjViewMatrix();
        return true;
    }

    ShadowCasterCascade::ShadowCasterCascade(Main &main, LightSceneNode *emitter, uint32_t cascadeNum, 
        uint32_t cascadeCount, float cascadeSplitLambda, uint32_t shadowMapSize, Camera &mainCamera) : 
        ShadowCaster(ShadowCaster::EmitterType::CascadeShadow, main, emitter),
        mCascadeNum(cascadeNum),
        mCascadeCount(cascadeCount),
        mShadowMapSize(shadowMapSize),
        mMainCamera(mainCamera)
    {
        SDL_assert(mLightNode->getLight()->getType() == LightSourceFlags::TypeDirectional);

        makeCascadeRange(cascadeSplitLambda, mMainCamera.getShadowClipNear(), mMainCamera.getShadowClipFar());
        mCamera = main.addCamera(mLightNode->getName() + "_shadow_cascade_" + std::to_string(cascadeNum) + "_" + 
            std::to_string(++gCameraCounter));

        mMainCamera.getRoot()->addObserver(this);
    }

    ShadowCasterCascade::~ShadowCasterCascade()
    {
        mMainCamera.getRoot()->removeObserver(this);
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

    float ShadowCasterCascade::splitDepth(uint32_t num, float lambda, float zNear, float zFar)
    {
        const float p = float(num + 1) / float(mCascadeCount);
        float logSplit = zNear * std::pow(zFar / zNear, p);
        float uniformSplit = zNear + (zFar - zNear) * p;

        return std::lerp(uniformSplit, logSplit, lambda);
    }

    void ShadowCasterCascade::makeCascadeRange(float lambda, float zNear, float zFar)
    {
        if (mCascadeNum == 0)
        {
            mCascadeNear = zNear;
            mCascadeFar = splitDepth(mCascadeNum, lambda, zNear, zFar);
            return;
        }

        mCascadeNear = splitDepth(mCascadeNum-1, lambda, zNear, zFar);
        mCascadeFar = splitDepth(mCascadeNum, lambda, zNear, zFar);
    }

    bool ShadowCasterCascade::recalcMatrix(kmMat4 &matrix)
    {
        /* получение главных осей камеры игрока в мировой системе координат */
        const auto forward = mMainCamera.getWorldDirection();
        const auto right = mMainCamera.getWorldRight();
        const auto up = mMainCamera.getWorldUp();
        const auto position = mMainCamera.getWorldPosition();
        const float midDepth = std::abs(mLightNode->getLight()->getShadowClipFar() - mLightNode->getLight()->getShadowClipNear()) / 2.0f;

        /* Ближняя и дальняя плоскость отсечения каскада в мировой системе координат */
        kmVec3 nc, fc;
        kmVec3Add(&nc, &position, kmVec3Scale(&nc, &forward, mCascadeNear));
        kmVec3Add(&fc, &position, kmVec3Scale(&fc, &forward, mCascadeFar));

        /* Размеры near/far плоскостей каскада */
        const float tanHalfFov = std::tan(mMainCamera.getFOVRad() * 0.5f);
        const float nearH = mCascadeNear * tanHalfFov;
        const float nearW = nearH * mMainCamera.getAspect();
        const float farH = mCascadeFar * tanHalfFov;
        const float farW = farH * mMainCamera.getAspect();

        /* Получение координат 8 углов каскада (frustum) в мировой системе координат */
        kmVec3 rightNear, upNear, rightFar, upFar, corners[8];

        kmVec3Scale(&rightNear, &right, nearW);
        kmVec3Scale(&upNear,    &up,    nearH);

        kmVec3Scale(&rightFar,  &right, farW);
        kmVec3Scale(&upFar,     &up,    farH);

        // near bottom-left
        kmVec3Subtract(&corners[0], &nc, &rightNear);
        kmVec3Subtract(&corners[0], &corners[0], &upNear);

        // near bottom-right
        kmVec3Add(&corners[1], &nc, &rightNear);
        kmVec3Subtract(&corners[1], &corners[1], &upNear);

        // near top-right
        kmVec3Add(&corners[2], &nc, &rightNear);
        kmVec3Add(&corners[2], &corners[2], &upNear);

        // near top-left
        kmVec3Subtract(&corners[3], &nc, &rightNear);
        kmVec3Add(&corners[3], &corners[3], &upNear);


        // far bottom-left
        kmVec3Subtract(&corners[4], &fc, &rightFar);
        kmVec3Subtract(&corners[4], &corners[4], &upFar);

        // far bottom-right
        kmVec3Add(&corners[5], &fc, &rightFar);
        kmVec3Subtract(&corners[5], &corners[5], &upFar);

        // far top-right
        kmVec3Add(&corners[6], &fc, &rightFar);
        kmVec3Add(&corners[6], &corners[6], &upFar);

        // far top-left
        kmVec3Subtract(&corners[7], &fc, &rightFar);
        kmVec3Add(&corners[7], &corners[7], &upFar);

        /* Найдем центральную точку каскада  */
        kmVec3 center, tmp;
        kmVec3Add(&center, &nc, &fc);
        kmVec3Scale(&center, &center, 0.5f);

        /* Найдем мнимую координату теневой камеры для каскада */
        kmVec3 shadowCamPosition;
        kmVec3Subtract(&shadowCamPosition, &center, kmVec3Scale(&tmp, &mLightNode->getLight()->getWorldDirection(), midDepth));

        /* Зададим View матрицу  */
        mCamera->setPosition(shadowCamPosition);
        mCamera->setDirection(mLightNode->getLight()->getWorldDirection());
        kmMat4 view = mCamera->refreshViewMatrix();

        /* Получим координаты углов каскада в системе координат теневой камеры (shadow-view-space) */
        float minX, minY, maxX, maxY;
        minX = minY = FLT_MAX;
        maxX = maxY = FLT_MIN;
        for (int i = 0; i < 8; ++i)
        {
            kmVec3TransformCoord(&tmp, &corners[i], &view);
        
            minX = std::min(minX, tmp.x);
            maxX = std::max(maxX, tmp.x);
        
            minY = std::min(minY, tmp.y);
            maxY = std::max(maxY, tmp.y);
        }

        /* расчет смещения для стабилизации вида, для уменешения дрожания теней (shimmering) */
        auto offset = calcStabilizationOffset(center, minX, maxX, minY, maxY);
        /* Построим ортогональную проекцию по полученным размерам каскада */
        mCamera->setupOrtho(
            mLightNode->getLight()->getShadowClipNear(), 
            mLightNode->getLight()->getShadowClipFar(),
            minX + offset.x, maxX + offset.x,
            minY + offset.y, maxY + offset.y);

        matrix = mCamera->refreshProjViewMatrix(view);
        return true;
    }

    kmVec2 ShadowCasterCascade::calcStabilizationOffset(const kmVec3 &center, float minX, float maxX, float minY, float maxY)
    {
        kmMat3 lightCamBasis;
        kmVec3 centerAtZero;
        kmMat3LookAt(&lightCamBasis, &KM_VEC3_ZERO, &mLightNode->getLight()->getWorldDirection(), &KM_VEC3_POS_Z);
        kmVec3MultiplyMat3(&centerAtZero, &center, &lightCamBasis);

        kmVec2 extent, texelSize, offset;
        extent.x = 0.5f * (maxX - minX);
        extent.y = 0.5f * (maxY - minY);
        texelSize.x = (extent.x * 2.0f) / mShadowMapSize;
        texelSize.y = (extent.y * 2.0f) / mShadowMapSize;
        offset.x = -centerAtZero.x + std::floor(centerAtZero.x / texelSize.x) * texelSize.x;
        offset.y = -centerAtZero.y + std::floor(centerAtZero.y / texelSize.y) * texelSize.y;
        return offset;
    }
}}
