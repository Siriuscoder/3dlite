/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
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

#include <string>
#include <algorithm>

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class SampleShadowManager : public RenderTargetObserver, public SceneObserver
{
public:

    using IndexVector = stl<uint32_t>::vector;

public:

    class ShadowCaster 
    {
    public:
        ShadowCaster(Main& main, LightSceneNode* node) : 
            mLightNode(node),
            mShadowCamera(main.addCamera(node->getName()))
        {
            // Ставим перспективу сразу при инициализации, считаем что конус источника света не меняется 
            mShadowCamera->setupPerspective(1.0f, mLightNode->getLight()->getInfluenceDistance(), 
                kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), 1.0);
        }

        kmMat4 getMatrix()
        {
            SDL_assert(mShadowCamera);
            // Обновим параметры теневой камеры
            mShadowCamera->setDirection(mLightNode->getLight()->getDirectionWorld());
            mShadowCamera->setPosition(mLightNode->getLight()->getPositionWorld());
            mShadowCamera->recalcFrustum();
            // Пересчитаем теневую матрицу
            return mShadowCamera->getProjTransformMatrix();
        }

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
            return mInvalideted;
        }

        inline void validate()
        {
            mInvalideted = false;
        }

        inline void invalidate()
        {
            mInvalideted = true;
        }

    private:

        LightSceneNode* mLightNode = nullptr;
        Camera* mShadowCamera = nullptr;
        bool mInvalideted = false;
    };

    class DynamicNode
    {
    public:
    
        using ShadowCasters = stl<ShadowCaster *>::vector;

        DynamicNode(SceneNode *node) : 
            mNode(node)
        {}

        void move(const kmVec3 &value)
        {
            SDL_assert(mNode);
            mNode->move(value);
        }

        void rotateAngle(const kmVec3 &axis, float angle)
        {
            SDL_assert(mNode);
            mNode->rotateAngle(axis, angle);
            invalidate();
        }

        void setPosition(const kmVec3 &pos)
        {
            SDL_assert(mNode);
            mNode->setPosition(pos);
            invalidate();
        }

        const kmVec3& getPosition() const
        {
            SDL_assert(mNode);
            return mNode->getPosition();
        }

        ShadowCasters& getVisibility()
        {
            return mVisibility;
        }

    private:

        void invalidate()
        {
            for (auto shadowCaster: mVisibility)
            {
                shadowCaster->invalidate();
            }
        }

        SceneNode *mNode = nullptr;
        ShadowCasters mVisibility;
    };

    SampleShadowManager(Main& main) : 
        mMain(main)
    {
        mShadowMatrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>("ShadowMatrixBuffer",
            "{\"Dynamic\": false}");
        mShadowIndexBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>("ShadowIndexBuffer",
            "{\"Dynamic\": true}");
    }

    ShadowCaster* newShadowCaster(LightSceneNode* node)
    {
        mShadowCasters.emplace_back(std::make_unique<ShadowCaster>(mMain, node));
        auto index = static_cast<uint32_t>(mShadowCasters.size() - 1);
        // Запишем в источник света индекс его теневой матрицы в UBO
        node->getLight()->setUserIndex(index);
        // Аллоцируем место под теневую матрицу 
        mShadowMatrixBuffer->extendBufferBytes(sizeof(kmMat4));
        return mShadowCasters.back().get();
    }

    DynamicNode* registerDynamicNode(SceneNode *node)
    {
        auto it = mDynamicNodes.emplace(node, DynamicNode(node));
        return &it.first->second;
    }

protected:

    bool beginUpdate(RenderTarget *rt) override
    { 
        mShadowRT = rt;
        SDL_assert(mShadowMatrixBuffer);
        mHostShadowIndexes.resize(1, 0); // Reserve 0 index for size
        // Обновим матрицы по всем источникам отбрасывающим тень которые влияют на текущий кадр
        for (uint32_t index = 0; index < mShadowCasters.size(); ++index)
        {
            auto &shadowCaster = mShadowCasters[index];
            if (shadowCaster->invalidated() && shadowCaster->getNode()->isVisible())
            {
                auto mat = shadowCaster->getMatrix();
                mShadowMatrixBuffer->setElement<kmMat4>(index, &mat);
                mHostShadowIndexes.emplace_back(index);
            }
        }

        mHostShadowIndexes[0] = static_cast<IndexVector::value_type>(mHostShadowIndexes.size()-1);
        // Если тени перересовывать не надо то просто переходим к следующией RT
        if (mHostShadowIndexes.size() == 1)
        {
            return false;
        }

        // Расширяем буфер индексов если надо
        if (mShadowIndexBuffer->bufferSizeBytes() < (mHostShadowIndexes.size() * sizeof(IndexVector::value_type)))
            mShadowIndexBuffer->extendBufferBytes((mHostShadowIndexes.size() * sizeof(IndexVector::value_type)) - mShadowIndexBuffer->bufferSizeBytes());
        
        mShadowIndexBuffer->setData(&mHostShadowIndexes[0], 0, mHostShadowIndexes.size() * sizeof(IndexVector::value_type));

        if (!mMainCamera)
        {
            mMainCamera = mMain.getCamera("MyCamera");
            SDL_assert(mMainCamera);
        }

        // Так как формально мы рендерим сцену от лица главной камеры, надо имменно для главной камеры на время рендера 
        // теневых карт включить отсечение лицевых граней
        mMainCamera->setCullFaceMode(Camera::CullFaceFront);

        return true;
    }

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool customVisibilityCheck(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera) override
    {
        auto it = mDynamicNodes.find(node);
        DynamicNode* dnode = nullptr;
        if (it != mDynamicNodes.end())
        {
            dnode = &it->second;
            dnode->getVisibility().clear();
        }

        bool isVisible = false;
        for (auto& shadowCaster: mShadowCasters)
        {
            if (shadowCaster->getCamera()->inFrustum(*boundingVol))
            {
                if (dnode)
                {
                    dnode->getVisibility().emplace_back(shadowCaster.get());
                }

                //if (shadowCaster->invalidated())
                {
                    isVisible = true;
                }
            }
        }

        return isVisible;
    }

    void postUpdate(RenderTarget *rt) override
    {
        // После рендера теневых карт возвращаем как было
        mMainCamera->setCullFaceMode(Camera::CullFaceBack);
        // Валидейтим только перересованные тени, остальные будут перерисованы потом когда попадут в область видимости
        for (auto index : mHostShadowIndexes)
        {
            mShadowCasters[index]->validate();
        }
    }

private:

    Main& mMain;
    Camera* mMainCamera = nullptr;
    RenderTarget* mShadowRT = nullptr;
    BufferBase* mShadowMatrixBuffer = nullptr;
    BufferBase* mShadowIndexBuffer = nullptr;
    IndexVector mHostShadowIndexes;
    stl<std::unique_ptr<ShadowCaster>>::vector mShadowCasters;
    stl<SceneNode *, DynamicNode>::unordered_map mDynamicNodes;
};

}}