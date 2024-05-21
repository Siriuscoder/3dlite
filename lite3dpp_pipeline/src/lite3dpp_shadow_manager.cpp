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
#include <lite3dpp_pipeline/lite3dpp_shadow_manager.h>

#include <algorithm>
#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    ShadowManager::ShadowCaster::ShadowCaster(Main& main, const String& name, LightSceneNode* node, 
        const lite3d_camera::projectionParamsStruct &params) : 
        mLightNode(node),
        mShadowCamera(main.addCamera(name))
    {
        SDL_assert(node);
        // Ставим перспективу сразу при инициализации, считаем что конус источника света не меняется 
        if (node->getLight()->getType() == LITE3D_LIGHT_DIRECTIONAL)
        {
            mShadowCamera->setupOrtho(params.znear, params.zfar, params.left, params.right, params.bottom, params.top);
        }
        else
        {
            mShadowCamera->setupPerspective(params.znear, params.zfar, 
                kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), params.aspect);
        }
    }

    kmMat4 ShadowManager::ShadowCaster::getMatrix()
    {
        SDL_assert(mShadowCamera);
        // Обновим параметры теневой камеры
        mShadowCamera->setDirection(mLightNode->getLight()->getWorldDirection());
        mShadowCamera->setPosition(mLightNode->getLight()->getWorldPosition());
        mShadowCamera->recalcFrustum();
        // Пересчитаем теневую матрицу
        return mShadowCamera->refreshProjViewMatrix();
    }

    void ShadowManager::DynamicShadowReceiver::move(const kmVec3 &value)
    {
        SDL_assert(mNode);
        mNode->move(value);
        invalidate();
    }

    void ShadowManager::DynamicShadowReceiver::rotateAngle(const kmVec3 &axis, float angle)
    {
        SDL_assert(mNode);
        mNode->rotateAngle(axis, angle);
        invalidate();
    }

    void ShadowManager::DynamicShadowReceiver::setPosition(const kmVec3 &pos)
    {
        SDL_assert(mNode);
        mNode->setPosition(pos);
        invalidate();
    }

    const kmVec3& ShadowManager::DynamicShadowReceiver::getPosition() const
    {
        SDL_assert(mNode);
        return mNode->getPosition();
    }

    void ShadowManager::DynamicShadowReceiver::clearVisibility()
    {
        mVisibility.clear();
    }

    void ShadowManager::DynamicShadowReceiver::addVisibility(ShadowCaster* sc)
    {
        if (std::find(mVisibility.begin(), mVisibility.end(), sc) == mVisibility.end())
        {
            mVisibility.emplace_back(sc);
        }
    }

    void ShadowManager::DynamicShadowReceiver::invalidate()
    {
        for (auto shadowCaster: mVisibility)
        {
            shadowCaster->invalidate();
        }
    }

    ShadowManager::ShadowManager(Main& main, const ConfigurationReader& conf) : 
        mMain(main)
    {
        int initialShadowCastersCount = conf.getInt(L"MaxCount", 1);
        int width = conf.getInt(L"Width", 1024);
        int height = conf.getInt(L"Height", 1024);
        mProjection.znear = conf.getInt(L"NearClipPlane", 1.0);
        mProjection.zfar = conf.getInt(L"FarClipPlane", 100.0);
        mProjection.left = conf.getObject(L"DirectionLightShadowParams").getInt(L"LeftClipPlane");
        mProjection.right = conf.getObject(L"DirectionLightShadowParams").getInt(L"RightClipPlane");
        mProjection.bottom = conf.getObject(L"DirectionLightShadowParams").getInt(L"BottomClipPlane");
        mProjection.top = conf.getObject(L"DirectionLightShadowParams").getInt(L"TopClipPlane");
        mProjection.aspect = static_cast<float>(width) / static_cast<float>(height);

        mShadowMatrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>("ShadowMatrixBuffer",
            "{\"Dynamic\": false}");
        mShadowIndexBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>("ShadowIndexBuffer",
            "{\"Dynamic\": true}");

        mShadowMatrixBuffer->extendBufferBytes(sizeof(kmMat4) * initialShadowCastersCount);
        mShadowIndexBuffer->extendBufferBytes(sizeof(IndexVector::value_type) * (initialShadowCastersCount + 1));
        IndexVector::value_type initialZero = 0;
        mShadowIndexBuffer->setElement<IndexVector::value_type>(0, &initialZero);
    }

    ShadowManager::ShadowCaster* ShadowManager::newShadowCaster(LightSceneNode* node)
    {
        auto index = static_cast<uint32_t>(mShadowCasters.size());
        mShadowCasters.emplace_back(std::make_unique<ShadowCaster>(mMain, node->getName() + std::to_string(index), 
            node, mProjection));
        // Запишем в источник света индекс его теневой матрицы в UBO
        node->getLight()->setUserIndex(index);
        // Аллоцируем место под теневую матрицу 
        if (mShadowMatrixBuffer->bufferSizeBytes() < mShadowCasters.size() * sizeof(kmMat4))
        {
            mShadowMatrixBuffer->extendBufferBytes(sizeof(kmMat4));
        }

        return mShadowCasters.back().get();
    }

    ShadowManager::DynamicShadowReceiver* ShadowManager::registerShadowReceiver(SceneNode *node)
    {
        auto it = mDynamicNodes.emplace(node, DynamicShadowReceiver(node));
        return &it.first->second;
    }

    bool ShadowManager::beginUpdate(RenderTarget *rt)
    { 
        mShadowRT = rt;
        SDL_assert(mShadowMatrixBuffer);

        mHostShadowIndexes.resize(1, 0); // Reserve 0 index for size
        // Обновим матрицы по всем источникам отбрасывающим тень которые влияют на текущий кадр
        for (uint32_t index = 0; index < mShadowCasters.size(); ++index)
        {
            auto &shadowCaster = mShadowCasters[index];
            auto mat = shadowCaster->getMatrix();
            if (shadowCaster->invalidated() && shadowCaster->getNode()->isVisible())
            {
                mShadowMatrixBuffer->setElement<kmMat4>(index, &mat);
                mHostShadowIndexes.emplace_back(index);
            }
        }

        mHostShadowIndexes[0] = static_cast<IndexVector::value_type>(mHostShadowIndexes.size()-1);
        // Если тени перересовывать не надо то просто переходим к следующией RT
        if (mHostShadowIndexes.size() == 1)
        {
            mShadowRT = nullptr;
            return false;
        }

        // Расширяем буфер индексов если надо
        if (mShadowIndexBuffer->bufferSizeBytes() < mHostShadowIndexes.size() * sizeof(IndexVector::value_type))
        {
            mShadowIndexBuffer->extendBufferBytes(mHostShadowIndexes.size() * sizeof(IndexVector::value_type) - 
                mShadowIndexBuffer->bufferSizeBytes());
        }

        mShadowIndexBuffer->setData(&mHostShadowIndexes[0], 0, mHostShadowIndexes.size() * sizeof(IndexVector::value_type));

        if (!mMainCamera)
        {
            mMainCamera = mMain.getCamera("MyCamera");
            SDL_assert(mMainCamera);
        }

        // Так как мы используем texture_array для хранения теневых карт мы в режиме layered render мы не можем подчистить
        // отдельную карту теней, а перерисовываем мы не все. Для очистки только нужных теневых карт используем предварительный 
        // проход с BigTriangle (сцена shadow_clean) устанавливающий во все фрагменты теневого буфера значение 1.0, но дело в том что его надо 
        // выполянть без проверки глубины, а при выключении ZTEST запись в буфер глубины невозможна, поэтому включаем 
        // ZTEST и устанавливаем TestFuncAlways для гарантированной перезаписи буфера грубины. Но перед рендером теней надо будет 
        // переключить обратно 
        RenderTarget::depthTestFunc(RenderTarget::TestFuncAlways);

        return true;
    }

    bool ShadowManager::beginSceneRender(Scene *scene, Camera *camera)
    { 
        // Только пока теневой RT активен
        if (mShadowRT)
        {
            RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
            // Подчистим списки источников света для которых эта нода видима перед проверкой фрустума.
            for (auto& node: mDynamicNodes)
            {
                node.second.clearVisibility();
            }
        }

        return true; 
    }

    // Проверим виден ли обьект сцены хотябы одной теневой камерой, если нет то рисовать его смысла нет.
    bool ShadowManager::customVisibilityCheck(Scene *scene, SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material, 
        lite3d_bounding_vol *boundingVol, Camera *camera)
    {
        auto it = mDynamicNodes.find(node);
        DynamicShadowReceiver* dnode = it != mDynamicNodes.end() ? &it->second : nullptr;

        bool isVisible = false;
        for (auto& shadowCaster: mShadowCasters)
        {
            if (shadowCaster->getCamera()->inFrustum(*boundingVol))
            {
                if (dnode)
                {
                    // Текущая нода видима для этого истоника света, запомним это
                    dnode->addVisibility(shadowCaster.get());
                }

                if (shadowCaster->invalidated())
                {
                    isVisible = true;
                }
            }
        }

        return isVisible;
    }

    void ShadowManager::postUpdate(RenderTarget *rt)
    {
        // Валидейтим только перересованные тени, остальные будут перерисованы потом когда попадут в область видимости
        for (size_t i = 1; i < mHostShadowIndexes.size(); ++i)
        {
            mShadowCasters[mHostShadowIndexes[i]]->validate();
        }

        mShadowRT = nullptr;
    }
}}
