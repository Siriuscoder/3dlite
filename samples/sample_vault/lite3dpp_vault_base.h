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

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class VaultBase : public Sample
{
public:
    
    VaultBase() :
        mGammaFactor(1.0f),
        mVaultScene(NULL),
        mAnimCounter(0),
        mAnimPi(0)
    {}
        
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

        mReactLamp01 = mVaultScene->getLightNode("ReactorLamp_01.node");
        mReactLamp02 = mVaultScene->getLightNode("ReactorLamp_02.node");
        mMinigun = mVaultScene->getObject("Minigun");
        mGatling = mVaultScene->getObject("Gatling");
        mLazer = mVaultScene->getObject("Lazer");
    }

    void mainCameraChanged() override
    {
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        mFlashLight->setPosition(getMainCamera().getPosition());
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
        mGatling->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        mGatling->getRoot()->setPosZ(-225 + soarDelta);
        
        mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        mMinigun->getRoot()->setPosZ(-225 + soarDelta);
        mMinigun->getNode("MinigunBurrel.node")->rotateAngle(KM_VEC3_POS_X, 0.23f);
        
        mLazer->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
        mLazer->getRoot()->setPosZ(-225 + soarDelta);
    }

    void addFlashlight(Scene *scene)
    {
        ConfigurationWriter flashlightJson;
        LightSource flashlight("FlashLight");
        flashlight.setAttenuationConstant(0.12f);
        flashlight.setAttenuationLinear(0.005f);
        flashlight.setAttenuationQuadratic(0.00002f);
        flashlight.setInfluenceDistance(1000.0f);
        flashlight.setAngleInnerCone(0.80f);
        flashlight.setAngleOuterCone(1.00f);
        flashlight.setDiffuse(KM_VEC3_ONE);
        flashlight.setDirection(KM_VEC3_NEG_Z);
        flashlight.setPosition(KM_VEC3_ZERO);
        flashlight.setType(LITE3D_LIGHT_SPOT);
        flashlight.toJson(flashlightJson);

        String flashLightParams = ConfigurationWriter().set(L"Name", "FlashLight.node").set(L"Light", flashlightJson).write();
        mFlashLight.reset(new LightSceneNode(ConfigurationReader(flashLightParams.data(), flashLightParams.size()), NULL, &getMain()));
        mFlashLight->addToScene(scene);
        mFlashLight->getLight()->enabled(false);
        mFlashLight->frustumTest(false);
    }
    
    virtual void lampsSwitchOn(bool flag)
    {
        for (const auto &light : mVaultScene->getLights())
        {
            if (light.first.find("Lamp_") != String::npos &&
                light.first.find("Spot") == String::npos &&
                light.first.find("Reactor") == String::npos)
            {
                light.second->getLight()->enabled(flag);
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
        else if (e->type == SDL_MOUSEMOTION)
        {
            mFlashLight->setRotation(getMainCamera().getRotation());
        }
    }
    
protected:
    
    float mGammaFactor;
    Scene *mVaultScene;
    float mAnimCounter;
    float mAnimPi;
    LightSceneNode *mReactLamp01;
    LightSceneNode *mReactLamp02;
    SceneObject *mMinigun;
    SceneObject *mGatling;
    SceneObject *mLazer;
    std::unique_ptr<LightSceneNode> mFlashLight;
};


}}

