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
#include <ctime>

#include "../sample_vault_111/lite3dpp_vault_shadows.h"
#include "../sample_vault_111/lite3dpp_vault_bloom.h"

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press '+' to increse gamma\n"
    "Press '-' to decrese gamma\n"
    "Press 'l' to enable/disable flashlight\n"
    "Press 'u' to enable/disable SSAO\n";

class SampleSponza : public Sample
{
public:

    SampleSponza() : 
        Sample(helpString)
    {
        // use current time as seed for random generator
        setCameraVelocityMax(0.15);
        setCameraAcceleration(0.02);
        setCameraResistance(0.01);
    }

    void createScene() override
    {
        mSponzaScene = getMain().getResourceManager()->queryResource<Scene>("Sponza", "sponza:scenes/sponza.json");
        
        setMainCamera(getMain().getCamera("MyCamera"));
        setupShadowCasters();
        addFlashlight();
    }

    void setupShadowCasters()
    {
       // mShadowManager->newShadowCaster(mSponzaScene->getObject("Sponza")->getLightNode("SUN"));
    }

    void addFlashlight()
    {
        auto flashLightObject = mSponzaScene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(1.0f);
        mFlashLight->getLight()->setAttenuationLinear(15.23f);
        mFlashLight->getLight()->setAttenuationQuadratic(35.23f);
        mFlashLight->getLight()->setRadiance(160.0f);
        mFlashLight->getLight()->enabled(false);
    }

    void frameBegin() override
    {
        updateFlashLight();
    }

    void updateFlashLight()
    {
        if (mFlashLight && mFlashLight->getLight()->enabled())
        {
            mFlashLight->setPosition(getMainCamera().getWorldPosition());
            mFlashLight->setRotation(getMainCamera().getWorldRotation());
        }
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_l && mFlashLight)
            {
                static bool flashLightEnabled = false;
                flashLightEnabled = !flashLightEnabled;
                mFlashLight->getLight()->enabled(flashLightEnabled);
                updateFlashLight();
            }
            else if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGammaFactor += 0.02;
                if (mGammaFactor > 3.0)
                    mGammaFactor = 3.0;
                Material::setFloatGlobalParameter("gamma", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGammaFactor -= 0.02;
                if (mGammaFactor < 1.5)
                    mGammaFactor = 1.5;
                Material::setFloatGlobalParameter("gamma", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_u)
            {
                static bool ssaoEnabled = true;
                ssaoEnabled = !ssaoEnabled;
                Material::setIntGlobalParameter("AOEnabled", ssaoEnabled ? 1 : 0);
                auto ssaoRenderTarget = getMain().getResourceManager()->queryResource<TextureRenderTarget>("SSAOStep");
                ssaoEnabled ? ssaoRenderTarget->enable() : ssaoRenderTarget->disable();
            }
        }
    }


private:

    lite3dpp::Scene* mSponzaScene = nullptr;
    LightSceneNode* mFlashLight;
    float mGammaFactor = 2.2;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleSponza sample;
    return sample.start("sponza_remaster/config/config.json");
}

