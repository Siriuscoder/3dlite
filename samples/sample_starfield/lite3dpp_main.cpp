/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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
    "Press '+' to increse brightness\n"
    "Press '-' to decrese brightness\n"
    "Press 'e' to go to the next weapoon\n"
    "Press 'r' to rotate weapoon\n";

class SampleStarfieldWeapons : public Sample
{
public:

    SampleStarfieldWeapons() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.015);
        setCameraAcceleration(0.003);
        setCameraResistance(0.001);
    }

    void createScene() override
    {
        mPipeline = getMain().getResourceManager().queryResource<lite3dpp_pipeline::PipelineForward>("Starfield", 
            "starfield:pipelines/starfield.json");
        mMainScene = &mPipeline->getMainScene();

        mAK47 = mMainScene->getObject("AK47");
        mVSS = mMainScene->getObject("VSS");

        mVSS->disable();
        
        setMainCamera(&mPipeline->getMainCamera());
        setupShadowCasters();
    }

    void setupShadowCasters()
    {
        mSUN = mMainScene->getObject("Sun")->getLightNode("Sun");
        mSUNShadowCaster = mPipeline->getShadowManager()->registerEmitter(mSUN);
        mPipeline->getShadowManager()->registerHintNode(mAK47->getRoot(), true);
        mPipeline->getShadowManager()->registerHintNode(mVSS->getRoot(), true);
    }

    void invalidateShadowCasters()
    {
        for (auto shadowCaster : mSUNShadowCaster)
            shadowCaster->invalidate();
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        if (mRotationEnabled)
        {
            if (mAK47->isEnabled())
            {
                mAK47->rotateZ(0.005f * deltaRetard);
            }
            else if (mVSS->isEnabled())
            {
                mVSS->rotateZ(0.005f * deltaRetard);
            }
        }
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_e)
            {
                if (mAK47->isEnabled())
                {
                    mAK47->disable();
                    mVSS->enable();
                    invalidateShadowCasters();
                }
                else if (mVSS->isEnabled())
                {
                    mAK47->enable();
                    mVSS->disable();
                    invalidateShadowCasters();
                }
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                mRotationEnabled = !mRotationEnabled;
            }
        }
    }


private:

    Scene* mMainScene = nullptr;
    SceneObject *mAK47 = nullptr;
    SceneObject *mVSS = nullptr;
    lite3dpp_pipeline::PipelineForward* mPipeline = nullptr;
    stl<lite3dpp_pipeline::ShadowCaster*>::vector mSUNShadowCaster;
    LightSceneNode* mSUN = nullptr;
    bool mRotationEnabled = false;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleStarfieldWeapons sample;
    return sample.start("starfield/config/config.json");
}

