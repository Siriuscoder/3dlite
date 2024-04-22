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

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class VaultBase : public Sample
{
public:
        
    virtual void createPipeline() = 0;

    void createScene() override
    {
        createPipeline();
        SDL_assert(mVaultScene);

        setMainCamera(getMain().getCamera("MainCamera"));
        
        // init flashlight
        addFlashlight(mVaultScene);
        
        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        lite3dpp::Material::setFloatv3GlobalParameter("screenResolution", resolution);
        lite3dpp::Material::setIntGlobalParameter("FXAA", 1);

        auto vaultObject = mVaultScene->getObject("Vault");
        mReactLamp01 = vaultObject->getLightNode("ReactorLamp_01.node");
        mReactLamp02 = vaultObject->getLightNode("ReactorLamp_02.node");
        mMinigun = mVaultScene->getObject("Minigun");
        mGatling = mVaultScene->getObject("Gatling");
        mLazer = mVaultScene->getObject("Lazer");
    }

    void mainCameraChanged() override
    {
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getWorldPosition());
        mFlashLight->setPosition(getMainCamera().getWorldPosition());
        mFlashLight->setRotation(getMainCamera().getWorldRotation());
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        float animCounterNew = mAnimCounter + (0.005f * deltaRetard);
        float animPiNew = mAnimPi + (0.23f * deltaRetard);

        mAnimCounter = animCounterNew >= 1.0f ? animCounterNew - mAnimCounter : animCounterNew;
        mAnimPi = animPiNew >= 2 * M_PI ? animPiNew - mAnimPi : animPiNew;

        lite3dpp::Material::setFloatGlobalParameter("animcounter", mAnimCounter);
        float attenuationConstant = (cos(mAnimPi) + 1.0f) / 2.0f;
        mReactLamp01->getLight()->setAttenuationConstant(attenuationConstant);
        mReactLamp01->getLight()->setAttenuationLinear(attenuationConstant / 100.0f);
        mReactLamp02->getLight()->setAttenuationConstant(attenuationConstant);
        mReactLamp02->getLight()->setAttenuationLinear(attenuationConstant / 100.0f);
        
        float soarDelta = cos(mAnimPi) * 5;
        mGatling->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        auto pos = mGatling->getPosition();
        pos.z = -225 + soarDelta;
        mGatling->setPosition(pos);
        
        mMinigun->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        pos = mMinigun->getPosition();
        pos.z = -225 + soarDelta;
        mMinigun->setPosition(pos);
        mMinigun->getNode("MinigunBurrel.node")->rotateAngle(KM_VEC3_POS_X, 0.23f);
        
        mLazer->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        pos = mLazer->getPosition();
        pos.z = -225 + soarDelta;
        mLazer->setPosition(pos);
    }

    void addFlashlight(Scene *scene)
    {
        auto flashLightObject = scene->addObject("FlashLight", "samples:objects/flashlight.json");
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(0.12f);
        mFlashLight->getLight()->setAttenuationLinear(0.005f);
        mFlashLight->getLight()->setAttenuationQuadratic(0.00002f);
        mFlashLight->getLight()->setInfluenceDistance(1000.0f);
        mFlashLight->getLight()->setDiffuse(KM_VEC3_ONE);
        mFlashLight->getLight()->enabled(false);
    }
    
    virtual void lampsSwitchOn(bool flag)
    {
        for (const auto &light : mVaultScene->getLights())
        {
            if (light->getName().find("Lamp_") != String::npos &&
                light->getName().find("Spot") == String::npos &&
                light->getName().find("Reactor") == String::npos)
            {
                light->getLight()->enabled(flag);
            }
        }
    }
    
    virtual void flashLightSwitchOn(bool flag)
    {
        mFlashLight->getLight()->enabled(flag);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGammaFactor += 0.02;
                if (mGammaFactor > 2.2)
                    mGammaFactor = 2.2;
                lite3dpp::Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGammaFactor -= 0.02;
                if (mGammaFactor < 1.0)
                    mGammaFactor = 1.0;
                lite3dpp::Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_o)
            {
                static bool fxaaEnabled = true;
                fxaaEnabled = !fxaaEnabled;
                lite3dpp::Material::setIntGlobalParameter("FXAA", fxaaEnabled ? 1 : 0);
            }
            else if (e->key.keysym.sym == SDLK_l)
            {
                static bool flashLightEnabled = false;
                flashLightEnabled = !flashLightEnabled;
                flashLightSwitchOn(flashLightEnabled);
            }
            else if (e->key.keysym.sym == SDLK_t)
            {
                static bool LampsEnabled = false;
                LampsEnabled = !LampsEnabled;
                lampsSwitchOn(LampsEnabled);
            }
        }
    }
    
protected:
    
    float mGammaFactor = 1.0f;
    Scene *mVaultScene = nullptr;
    float mAnimCounter = 0.0f;
    float mAnimPi = 0.0f;
    LightSceneNode *mReactLamp01 = nullptr;
    LightSceneNode *mReactLamp02 = nullptr;
    SceneObject *mMinigun = nullptr;
    SceneObject *mGatling = nullptr;
    SceneObject *mLazer = nullptr;
    LightSceneNode *mFlashLight = nullptr;
};


}}

