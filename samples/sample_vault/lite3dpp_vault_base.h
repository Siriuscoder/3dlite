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
    
    void timerTick(lite3d_timer *timerid) override
    {
        Sample::timerTick(timerid);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        mFlashLight->setPosition(getMainCamera().getPosition());

        if (timerid == getMain().getFixedUpdateTimer())
        {
            mAnimCounter = mAnimCounter >= 1.0f ? 0.0f : mAnimCounter + 0.005f;
            mAnimPi = mAnimPi >= 2 * M_PI ? 0.0f : mAnimPi + 0.23f;
            
            lite3dpp::Material::setFloatGlobalParameter("animcounter", mAnimCounter);

            kmVec4 attenuation = mReactLamp01->getLight()->getAttenuation();
            attenuation.x = (cos(mAnimPi) + 1) / 2;
            attenuation.y = attenuation.x / 100;
            mReactLamp01->getLight()->setAttenuation(attenuation);
            mReactLamp02->getLight()->setAttenuation(attenuation);
            
            float soarDelta = cos(mAnimPi) * 5;
            mGatling->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mGatling->getRoot()->setPosZ(-225 + soarDelta);
            
            mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mMinigun->getRoot()->setPosZ(-225 + soarDelta);
            mMinigun->getNode("MinigunBurrel.node")->rotateAngle(KM_VEC3_POS_X, 0.23f);
            
            mLazer->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mLazer->getRoot()->setPosZ(-225 + soarDelta);
        }
    }

    void addFlashlight(Scene *scene)
    {
        kmVec3 spotFactor = { 0.80f, 1.0f, 0.0f };
        kmVec4 attenuation = { 0.12, 0.005, 0.00002, 1000.0f };
        String flashLightParams = ConfigurationWriter().set(L"Name", "FlashLight.node").set(L"Light", 
            ConfigurationWriter().set(L"Ambient", KM_VEC3_ZERO)
            .set(L"Diffuse", KM_VEC3_ONE)
            .set(L"Position", KM_VEC3_ZERO)
            .set(L"Name", "FlashLight")
            .set(L"Specular", KM_VEC3_ONE)
            .set(L"SpotDirection", KM_VEC3_NEG_Z)
            .set(L"Type", "Spot")
            .set(L"SpotFactor", spotFactor)
            .set(L"Attenuation", attenuation)).write();

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

