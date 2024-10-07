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
    "Press '+' to increse gamma\n"
    "Press '-' to decrese gamma\n"
    "Press 'l' to enable/disable flashlight\n"
    "Press 'u' to enable/disable SSAO\n"
    "Press 'r' to enable/disable night mode\n"
    "Press 'e' to enable/disable sun rotation\n";

class SampleSponza : public Sample
{
public:

    SampleSponza() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.15);
        setCameraAcceleration(0.02);
        setCameraResistance(0.01);
    }

    void createScene() override
    {
        mPipeline = getMain().getResourceManager()->queryResource<lite3dpp_pipeline::PipelineDeffered>("SponzaDeffered", 
            "sponza:pipelines/sponza.json");
        mSponzaScene = &mPipeline->getMainScene();
        
        setMainCamera(&mPipeline->getMainCamera());
        setupShadowCasters();
        addFlashlight();
    }

    void setupShadowCasters()
    {
        mSUN = mSponzaScene->getObject("Sponza")->getLightNode("SUN");
        mAmbient01 = mSponzaScene->getObject("Sponza")->getLightNode("ambient_light_day_01");
        mAmbient02 = mSponzaScene->getObject("Sponza")->getLightNode("ambient_light_day_02");
        mSUNNode = mSponzaScene->getObject("Sponza")->getNode("SUN_actor");
        mSUNShadowCaster = mPipeline->getShadowManager()->newShadowCaster(mSUN);
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        if (mDayNightMode && mSunRotation)
        {
            // Крутим источник света
            mSUNNode->rotateZ(0.0005f * deltaRetard);
            //mSUN->rotateX(0.00005f * deltaRetard);
            // Помечаем что надо перерисовать тени в следубщий кадр
            mSUNShadowCaster->invalidate();
        }
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
            else if (e->key.keysym.sym == SDLK_u)
            {
                static bool ssaoEnabled = true;
                ssaoEnabled = !ssaoEnabled;
                mPipeline->enableSSAO(ssaoEnabled);
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                mDayNightMode = !mDayNightMode;
                mSUN->getLight()->enabled(mDayNightMode);
                mAmbient01->getLight()->enabled(mDayNightMode);
                mAmbient02->getLight()->enabled(mDayNightMode);
                mPipeline->setSkyBoxEmission(mDayNightMode ? 12.0f : 0.008f);
                mPipeline->getIBL()->rebuild();
            }
            else if (e->key.keysym.sym == SDLK_e)
            {
                mSunRotation = !mSunRotation;
            }
        }
    }


private:

    Scene* mSponzaScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    lite3dpp_pipeline::ShadowManager::ShadowCaster *mSUNShadowCaster = nullptr;
    LightSceneNode* mFlashLight = nullptr;
    LightSceneNode* mSUN = nullptr;
    LightSceneNode* mAmbient01 = nullptr;
    LightSceneNode* mAmbient02 = nullptr;
    SceneNode* mSUNNode = nullptr;
    bool mDayNightMode = true;
    bool mSunRotation = false;
    float mGamma = 2.2;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleSponza sample;
    return sample.start("sponza/config/config.json");
}

