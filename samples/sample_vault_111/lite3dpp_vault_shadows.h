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

#include <iostream>
#include <string>

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class SampleShadowManager : public RenderTargetObserver
{
public:

    class ShadowCaster 
    {
    public:
        ShadowCaster(Main& main, RenderTarget* rt, LightSceneNode* node) : 
            mLightNode(node),
            mShadowCamera(main.addCamera(node->getName()))
        {}

        kmMat4 getMatrix()
        {
            SDL_assert(mShadowCamera);
            SDL_assert(mLightNode);
            // Обновим глобальные координаты источника света
            mLightNode->translateToWorld();
            // Обновим параметры теневой камеры
            mShadowCamera->setDirection(mLightNode->getLight()->getDirectionWorld());
            mShadowCamera->setPosition(mLightNode->getLight()->getPositionWorld());
            mShadowCamera->setupPerspective(10.0f, 4000.0f, 
                kmRadiansToDegrees(mLightNode->getLight()->getAngleOuterCone()), 1.0);
            // Пересчитаем теневую камеру
            return mShadowCamera->getProjTransformMatrix();
        }

        void rotateAngle(const kmVec3 &axis, float angle)
        {
            SDL_assert(mLightNode);
            SDL_assert(mShadowRT);
            mLightNode->rotateAngle(axis, angle);
            // После обновления ориентации источника света надо перерисовать тени
            mShadowRT->enable();
        }

    private:

        LightSceneNode* mLightNode = nullptr;
        Camera* mShadowCamera = nullptr;
        RenderTarget* mShadowRT = nullptr;
    };

    SampleShadowManager(Main& main) : 
        mMain(main)
    {
        mShadowMatrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>("ShadowMatrixBuffer",
            "{\"Dynamic\": false}");
    }

    bool beginUpdate(RenderTarget *rt) override
    { 
        // Обновим матрицы по всем источникам отбрасывающим тень.
        for (uint32_t index = 0; index < mShadowCasters.size(); ++index)
        {
            mShadowMatrixBuffer->setElement<kmMat4>(index, &mShadowCasters[index]->getMatrix());
        }

        return true;
    }

    void postUpdate(RenderTarget *rt) override
    {
        // Отключим рендер теней после обновления всех теней, результаты будут валидны до тех пор пока ориентация и позиция
        // источника света и обьектов отбрасывающих тень не изменится
        rt->disable();
    }

    ShadowCaster* newShadowCaster(RenderTarget* rt, LightSceneNode* node)
    {
        SDL_assert(mShadowMatrixBuffer);
        mShadowCasters.emplace_back(std::make_unique<ShadowCaster>(mMain, rt, node));
        auto index = static_cast<uint32_t>(mShadowCasters.size() - 1);
        // Обновим UBO с теневыми матрицами
        mShadowMatrixBuffer->setElement<kmMat4>(index, &mShadowCasters.back()->getMatrix());
        // Запишем в источник света индекс его теневой матрицы в UBO
        node->getLight()->setUserIndex(index);
        return mShadowCasters.back().get();
    }

private:

    Main& mMain;
    BufferBase* mShadowMatrixBuffer = nullptr;
    std::vector<std::unique_ptr<ShadowCaster>> mShadowCasters;
};

}}