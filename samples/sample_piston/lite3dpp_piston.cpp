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
    "Press 'u' to enable/disable SSAO\n"
    "Press 'q' to start animation\n"
    "Press 'e' to pause/resume animation\n"
    "Press 'r' to reset animation\n";

class SamplePiston : public Sample
{
public:

    SamplePiston() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.15);
        setCameraAcceleration(0.02);
        setCameraResistance(0.01);
    }

    void createScene() override
    {
        mPipeline = getMain().getResourceManager()->queryResource<lite3dpp_pipeline::PipelineDeffered>("PistonScene", 
            "piston:pipelines/piston.json");
        mPistonScene = &mPipeline->getMainScene();
        
        setMainCamera(&mPipeline->getMainCamera());
        setupShadowCasters();

        mCrankshaft = mPistonScene->getObject("Engine")->getNode("Crankshaft");
        mPiston = mPistonScene->getObject("Engine")->getNode("Piston");
        mRod = mPistonScene->getObject("Engine")->getNode("Rod");

        getMain().getResourceManager()->warmUpMeshPartitions();
        getMain().getResourceManager()->dropFileCache();
    }

    void setupShadowCasters()
    {
        auto sun = mPistonScene->getObject("Ground")->getLightNode("Sun");
        sun->getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);
        mSUNShadowCaster = mPipeline->getShadowManager()->newShadowCaster(sun);
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        // Если анимация запущена, то перерисуем тени
        if (mCrankshaft->getActionState() == ActionClip::ActionClipState::PLAYING)
            mSUNShadowCaster->invalidate();
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
            else if (e->key.keysym.sym == SDLK_q)
            {
                mCrankshaft->actionPlayCycle("Crankshaft.action");
                mRod->actionPlayCycle("Rod.action");
                mPiston->actionPlayCycle("Piston.action");
            }
            else if (e->key.keysym.sym == SDLK_e)
            {
                if (mCrankshaft->getActionState() == ActionClip::ActionClipState::PLAYING)
                {
                    mCrankshaft->actionPause();
                    mRod->actionPause();
                    mPiston->actionPause();
                }
                else
                {
                    mCrankshaft->actionResume();
                    mRod->actionResume();
                    mPiston->actionResume();
                }
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                mCrankshaft->actionReset();
                mRod->actionReset();
                mPiston->actionReset();
            }
        }
    }


private:

    Scene* mPistonScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    lite3dpp_pipeline::ShadowManager::ShadowCaster *mSUNShadowCaster = nullptr;
    SceneNode *mCrankshaft = nullptr;
    SceneNode *mRod = nullptr;
    SceneNode *mPiston = nullptr;
    float mGamma = 2.2;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SamplePiston sample;
    return sample.start("piston/config/config.json");
}

