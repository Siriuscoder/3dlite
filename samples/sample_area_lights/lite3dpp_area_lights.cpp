/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <sample_common/lite3dpp_common.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline.h>

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press '+' to increse gamma\n"
    "Press '-' to decrese gamma\n"
    "Press 'u' to enable/disable SSAO\n";

class SampleAreaLights : public Sample
{
public:

    static constexpr const float AttenuationConstant = 1.0;
    static constexpr const float AttenuationLinear = 15.23f;
    static constexpr const float AttenuationQuadratic = 35.23f;

    SampleAreaLights() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.15);
        setCameraAcceleration(0.02);
        setCameraResistance(0.01);
    }

    void createScene() override
    {
        mPipeline = getMain().getResourceManager().queryResource<lite3dpp_pipeline::PipelineDeffered>("AreaLightsSample", 
            "area_lights:pipelines/area_lights.json");
        mScene = &mPipeline->getMainScene();
        
        setMainCamera(&mPipeline->getMainCamera());
        addFlashlight();
    }

    void mainCameraChanged() override
    {
        Sample::mainCameraChanged();
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

    void addFlashlight()
    {
        auto flashLightObject = mScene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->setPosition(getMainCamera().getPosition());
        mFlashLight->setRotation(getMainCamera().getRotation());
        mFlashLight->getLight()->setAttenuationConstant(AttenuationConstant);
        mFlashLight->getLight()->setAttenuationLinear(AttenuationLinear);
        mFlashLight->getLight()->setAttenuationQuadratic(AttenuationQuadratic);
        mFlashLight->getLight()->setRadiance(700.0f);
        mFlashLight->getLight()->enabled(true);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGamma += 0.02;
                if (mGamma > 3.0)
                    mGamma = 3.0;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGamma -= 0.02;
                if (mGamma < 1.5)
                    mGamma = 1.5;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.keysym.sym == SDLK_u)
            {
                static bool ssaoEnabled = true;
                ssaoEnabled = !ssaoEnabled;
                mPipeline->enableSSAO(ssaoEnabled);
            }
            else if (e->key.keysym.sym == SDLK_l && mFlashLight)
            {
                static bool flashLightEnabled = true;
                flashLightEnabled = !flashLightEnabled;
                mFlashLight->getLight()->enabled(flashLightEnabled);
                
                updateFlashLight();
            }
        }
    }

private:

    Scene* mScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    LightSceneNode* mFlashLight = nullptr;
    float mGamma = 2.2;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleAreaLights sample;
    return sample.start("area_lights/config/config.json");
}

