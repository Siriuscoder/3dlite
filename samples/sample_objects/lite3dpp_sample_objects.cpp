/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <iostream>

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>

class SampleLifecycleListener : public lite3dpp::Main::LifecycleListener
{
public:

    SampleLifecycleListener(lite3dpp::Main *main) : 
        mMain(main),
        mMinigun(NULL),
        mPlasmagun(NULL)
    {}

    void init() override
    {
        lite3dpp::Scene *scene = mMain->getResourceManager()->queryResource<lite3dpp::Scene>("SimpleScene",
            "samples:scenes/scene.json");
        mMinigun = scene->getObject("Minigun");
        mPlasmagun = scene->getObject("Plasmagun");
    }

    void shut() override
    {}

    void frameBegin() override
    {}

    void frameEnd() override
    {}

    void timerTick(lite3d_timer *timerid) override
    {
        mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.01f);
        mPlasmagun->getRoot()->rotateAngle(KM_VEC3_NEG_Z, 0.01f);
    }

    void processEvent(SDL_Event *e) override
    {
        if (e->type == SDL_KEYDOWN)
        {
            /* exit */
            if (e->key.keysym.sym == SDLK_ESCAPE)
                mMain->stop();
        }
        else if (e->key.keysym.sym == SDLK_F1)
        {
            lite3d_render_stats *stats = lite3d_render_stats_get();
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "==== Render statistics ========\n"
                "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
                "%d\t\t%d\t\t%d\t\t%d\n"
                "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
                "%f\t%f\t%f\t%f",
                stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
                stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs);
        }
    }

private:

    lite3dpp::Main *mMain;
    lite3dpp::SceneObject *mMinigun;
    lite3dpp::SceneObject *mPlasmagun;
};

int main(int agrc, char *args[])
{
    try
    {
        lite3dpp::Main mainObj;
        SampleLifecycleListener lifecycleListener(&mainObj);

        mainObj.registerLifecycleListener(&lifecycleListener);
        mainObj.initFromConfig("samples/config/config.json");
        mainObj.run();
    }
    catch (std::exception &ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
