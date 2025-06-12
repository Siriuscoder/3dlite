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
    "Press 'z' to start second skeleton body animation\n"
    "Press 'e' to pause/resume all animation\n"
    "Press 'r' to reset all animation\n";

class SampleAnimation : public Sample
{
public:

    SampleAnimation() : 
        Sample(helpString)
    {
        setCameraVelocityMax(0.15);
        setCameraAcceleration(0.02);
        setCameraResistance(0.01);
    }

    void createScene() override
    {
        mPipeline = getMain().getResourceManager().queryResource<lite3dpp_pipeline::PipelineDeffered>("AnimationScene", 
            "animation:pipelines/animation.json");
        mAnimationScene = &mPipeline->getMainScene();
        
        setMainCamera(&mPipeline->getMainCamera());

        mCrankshaft = mAnimationScene->getObject("Engine")->getNode("Crankshaft");
        mPiston = mAnimationScene->getObject("Engine")->getNode("Piston");
        mRod = mAnimationScene->getObject("Engine")->getNode("Rod");
        mSkeletonBody = mAnimationScene->getObject("SkeletonBody")->getRoot();
        mSkeletonBody2 = mAnimationScene->getObject("SkeletonBody2")->getRoot();
        mCuriousSphere = mAnimationScene->getObject("CuriousSphere")->getRoot();

        setupShadowCasters();
        getMain().getResourceManager().warmUpMeshPartitions();
        getMain().getResourceManager().dropFileCache();
    }

    void setupShadowCasters()
    {
        auto sun = mAnimationScene->getObject("Ground")->getLightNode("Sun");
        sun->getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);
        mSUNShadowCaster = mPipeline->getShadowManager()->newShadowCaster(sun);
        // Register hint nodes for automate shadow recalculation 
        mPipeline->getShadowManager()->registerHintNodeRecursive(mAnimationScene->getObject("Engine")->getRoot());
        mPipeline->getShadowManager()->registerHintNode(mSkeletonBody);
        mPipeline->getShadowManager()->registerHintNode(mSkeletonBody2);
        mPipeline->getShadowManager()->registerHintNode(mCuriousSphere);
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
                mCrankshaft->actionPlay("Crankshaft.action", true);
                mRod->actionPlay("Rod.action", true);
                mPiston->actionPlay("Piston.action", true);
                mPiston->actionPlay("Piston.action", true);
                mSkeletonBody->actionPlay("SkeletonBodyAction.action", true);
            }
            else if (e->key.keysym.sym == SDLK_z)
            {
                mSkeletonBody2->actionPlay("SkeletonBodyAction.action", true);
            }
            else if (e->key.keysym.sym == SDLK_x)
            {
                mCuriousSphere->actionPlay("CuriousSphereAction.action", true);
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

                if (mSkeletonBody->getActionState() == ActionClip::ActionClipState::PLAYING)
                {
                    mSkeletonBody->actionPause();
                }
                else
                {
                    mSkeletonBody->actionResume();
                }

                if (mSkeletonBody2->getActionState() == ActionClip::ActionClipState::PLAYING)
                {
                    mSkeletonBody2->actionPause();
                }
                else
                {
                    mSkeletonBody2->actionResume();
                }

                if (mCuriousSphere->getActionState() == ActionClip::ActionClipState::PLAYING)
                {
                    mCuriousSphere->actionPause();
                }
                else
                {
                    mCuriousSphere->actionResume();
                }
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                mCrankshaft->actionReset();
                mRod->actionReset();
                mPiston->actionReset();
                mSkeletonBody->actionReset();
                mSkeletonBody2->actionReset();
                mCuriousSphere->actionReset();
            }
        }
    }


private:

    Scene* mAnimationScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    lite3dpp_pipeline::ShadowManager::ShadowCaster *mSUNShadowCaster = nullptr;
    SceneNodeBase *mCrankshaft = nullptr;
    SceneNodeBase *mRod = nullptr;
    SceneNodeBase *mPiston = nullptr;
    SceneNodeBase *mSkeletonBody = nullptr;
    SceneNodeBase *mSkeletonBody2 = nullptr;
    SceneNodeBase *mCuriousSphere = nullptr;
    float mGamma = 2.2;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleAnimation sample;
    return sample.start("animation/config/config.json");
}

