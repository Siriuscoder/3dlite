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
#include <sample_common/lite3dpp_common.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline.h>

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press 'l' to enable/disable flashlight\n"
    "Press 'r' to add light spark\n"
    "Press 't' to add spot light\n";

class SampleSandbox : public Sample
{
public:

    static constexpr const float AttenuationConstant = 1.0;
    static constexpr const float AttenuationLinear = 15.23f;
    static constexpr const float AttenuationQuadratic = 35.23f;

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

        // Player position is the Zero probe
        mPipeline->getIBL()->addProbe(getMainCamera().getPosition());

        kmVec3 probePos = { -2.0, -4.0, 2.0 };
        for (int i = 0; i < 2; ++i)
        {
            for (int i = 0; i < 2; ++i)
            {
                for (int i = 0; i < 3; ++i)
                {
                    mPipeline->getIBL()->addProbe(probePos); 
                    probePos.y += 4.0;
                }

                probePos.x += 4.0;
                probePos.y = -4.0;
            }

            probePos.x = -2.0;
            probePos.z += 2.0;
        }
    }

    void mainCameraChanged() override
    {
        Sample::mainCameraChanged();
        updateFlashLight();

        // Update zero probe, set current position
        mPipeline->getIBL()->updateProbe(0, getMainCamera().getWorldPosition());
    }

    void addFlashlight()
    {
        auto flashLightObject = mMainScene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(AttenuationConstant);
        mFlashLight->getLight()->setAttenuationLinear(AttenuationLinear);
        mFlashLight->getLight()->setAttenuationQuadratic(AttenuationQuadratic);
        mFlashLight->getLight()->setRadiance(100.0f);
        mFlashLight->getLight()->enabled(false);
    }

    void addSpotLight()
    {
        auto flashLightObject = mMainScene->addObject("SpotLight_" + std::to_string(++mObjectCounter), 
            "samples:objects/flashlight.json", nullptr);
        auto spotLight = flashLightObject->getLightNode("FlashLight.node");
        spotLight->getLight()->setAttenuationConstant(AttenuationConstant);
        spotLight->getLight()->setAttenuationLinear(AttenuationLinear);
        spotLight->getLight()->setAttenuationQuadratic(AttenuationQuadratic);
        spotLight->getLight()->setRadiance(200.0f);
        spotLight->getLight()->enabled(true);
        spotLight->setPosition(getMainCamera().getWorldPosition());
        spotLight->setRotation(getMainCamera().getWorldRotation());

        // Recalc global illumination
        mPipeline->getIBL()->rebuild();
    }

    void addSpark()
    {
        auto sparkObject = mMainScene->addObject("Spark_" + std::to_string(++mObjectCounter), 
            "samples:objects/light_spark.json", nullptr, getMainCamera().getWorldPosition());
        auto node = sparkObject->getLightNode("PointLightSpark.node");
        node->getLight()->setAttenuationConstant(AttenuationConstant);
        node->getLight()->setAttenuationLinear(AttenuationLinear);
        node->getLight()->setAttenuationQuadratic(AttenuationQuadratic);
        node->getLight()->setRadiance(60.0f);
        node->getLight()->enabled(true);

        // Recalc global illumination
        mPipeline->getIBL()->rebuild();
    }

    void updateFlashLight()
    {
        if (mFlashLight && mFlashLight->getLight()->enabled())
        {
            mFlashLight->setPosition(getMainCamera().getWorldPosition());
            mFlashLight->setRotation(getMainCamera().getWorldRotation());

            // Recalc global illumination
            mPipeline->getIBL()->rebuild();
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
                // Recalc global illumination
                mPipeline->getIBL()->rebuild();
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
            else if (e->key.keysym.sym == SDLK_t)
            {
                addSpotLight();
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
