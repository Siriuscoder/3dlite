/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <iostream>

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>

class SampleLifecycleListener : public lite3dpp::Main::LifecycleListener
{
public:

    SampleLifecycleListener(lite3dpp::Main *main) : 
        mMain(main),
        mMode(1)
    {}

    void init() override
    {
        lite3dpp::Scene *scene = mMain->getResourceManager()->queryResource<lite3dpp::Scene>("Vault",
            "vaultmat:scenes/vault.json");
        mCamera = scene->getCamera("MyCamera");
        mWindow = mMain->window();
        
        mCenterXPos = mWindow->width() >> 1;
        mCenterYPos = mWindow->height() >> 1;
        lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);

        mStatRerfeshTimer = mMain->addTimer("statisticURefresh", 1000);
        lite3dpp::Material::setFloatGlobalParameter("mode", 2);

        mMain->getResourceManager()->releaseFileCache();
    }

    void shut() override
    {}

    void frameBegin() override
    {}

    void frameEnd() override
    {}

    void timerTick(lite3d_timer *timerid) override
    {
        if(timerid == mMain->getFixedUpdateTimer())
        {
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if(state[SDL_SCANCODE_W])
            {
                kmVec3 vec3 = {0, 0, -6};
                mCamera->moveRelative(vec3);
            }
            
            if(state[SDL_SCANCODE_S])
            {
                kmVec3 vec3 = {0, 0, 6};
                mCamera->moveRelative(vec3);
            }
            
            if(state[SDL_SCANCODE_A])
            {
                kmVec3 vec3 = {-6, 0, 0};
                mCamera->moveRelative(vec3);
            }
            
            if(state[SDL_SCANCODE_D])
            {
                kmVec3 vec3 = {6, 0, 0};
                mCamera->moveRelative(vec3);
            }
        }
        else if(timerid == mStatRerfeshTimer)
        {
            printStats();
        }
    }

    void processEvent(SDL_Event *e) override
    {
        if (e->type == SDL_KEYDOWN)
        {
            /* exit */
            if (e->key.keysym.sym == SDLK_ESCAPE)
                mMain->stop();
            else if (e->key.keysym.sym == SDLK_F1)
            {
                printStats();
            }
            else if (e->key.keysym.sym == SDLK_m)
            {
                if (mMode == 1)
                    mMode = 2;
                else 
                    mMode = 1;
                
                lite3dpp::Material::setFloatGlobalParameter("mode", (float)mMode);
            }
            else if (e->key.keysym.sym == SDLK_f)
            {
                static bool scRes = true;
                if(scRes)
                {
                    mWindow->resize(1024, 768);
                    mWindow->fullscreen(false);
                }
                else
                {
                    mWindow->resize(0, 0);
                    mWindow->fullscreen(true);
                }

                mCenterXPos = mWindow->width() >> 1;
                mCenterYPos = mWindow->height() >> 1;
                lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
                mCamera->setAspect(mWindow->computeCameraAspect());
                scRes = !scRes;
            }
        }
        else if(e->type == SDL_MOUSEMOTION)
        {
            mCamera->rotateZ((e->motion.x - mCenterXPos) * 0.003f);
            mCamera->pitch((e->motion.y - mCenterYPos) * 0.003f);
            lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
        }
    }

    void printStats()
    {
        lite3d_render_stats *stats = lite3d_render_stats_get();
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "==== Render statistics ========\n"
            "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
            "%d\t\t%d\t\t%d\t\t%d\n"
            "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
            "%f\t%f\t%f\t%f\n"
            "nodes total\tbatches total\tbatches called\tfaces\n"
            "%d\t\t%d\t\t%d\t\t%d\n",
            stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
            stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs,
            stats->nodesTotal, stats->batchesTotal, stats->batchedByFrame, stats->verticesByFrame);
    }

private:

    lite3dpp::Main *mMain;
    lite3dpp::Camera *mCamera;
    lite3dpp::WindowRenderTarget *mWindow;
    int mCenterXPos;
    int mCenterYPos;
    lite3d_timer *mStatRerfeshTimer;
    int mMode;
};

int main(int agrc, char *args[])
{
    try
    {
        lite3dpp::Main mainObj;
        SampleLifecycleListener lifecycleListener(&mainObj);

        mainObj.registerLifecycleListener(&lifecycleListener);
        mainObj.initFromConfig("vault/config/config_vault.json");
        mainObj.run();
    }
    catch (std::exception &ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
