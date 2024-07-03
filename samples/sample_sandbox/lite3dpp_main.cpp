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

#include <sample_common/lite3dpp_common.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline.h>

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press 'l' to enable/disable flashlight\n"
    "Press 'r' to add light spark\n";

class SampleSandbox : public Sample
{
public:

    SampleSandbox() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.035);
        setCameraAcceleration(0.010);
        setCameraResistance(0.005);
    }

    void createScene() override
    {
        // load empty scene
        mPipeline = getMain().getResourceManager()->queryResource<lite3dpp_pipeline::PipelineForward>("SandboxPipeline", 
            "sandbox:pipelines/sandbox.json");
        mMainScene = &mPipeline->getMainScene();

        setMainCamera(&mPipeline->getMainCamera());
        addFlashlight();
    }

    void frameBegin() override
    {
        updateFlashLight();
    }

    void addFlashlight()
    {
        auto flashLightObject = mMainScene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(1.0f);
        mFlashLight->getLight()->setAttenuationLinear(15.23f);
        mFlashLight->getLight()->setAttenuationQuadratic(35.23f);
        mFlashLight->getLight()->setRadiance(100.0f);
        mFlashLight->getLight()->enabled(false);
    }

    void addSpark()
    {
        auto sparkObject = mMainScene->addObject("Spark_" + std::to_string(++mObjectCounter), 
            "samples:objects/light_spark.json", nullptr, getMainCamera().getWorldPosition());
        auto node = sparkObject->getLightNode("PointLightSpark.node");
        node->getLight()->setAttenuationConstant(1.0f);
        node->getLight()->setAttenuationLinear(15.23f);
        node->getLight()->setAttenuationQuadratic(35.23f);
        node->getLight()->setRadiance(60.0f);
        node->getLight()->enabled(true);
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
            else if (e->key.keysym.sym == SDLK_r)
            {
                addSpark();
            }
        }
    }
    
private:

    Scene* mMainScene = nullptr;
    lite3dpp_pipeline::PipelineForward* mPipeline = nullptr;
    LightSceneNode* mFlashLight = nullptr;
    float mGamma = 2.2f;
    int mObjectCounter = 0;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleSandbox sample;
    return sample.start("sandbox/config/config.json");
}
