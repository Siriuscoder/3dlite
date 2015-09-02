/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#include <3dlitepp/3dlitepp_main.h>


class SampleLifecycleListener : public lite3dpp::Main::LifecycleListener
{
public:

    SampleLifecycleListener()
    {
        mCameraPos.x = mCameraPos.y = mCameraPos.z = 80.0f;
        mCameraLookAt.x = mCameraLookAt.y = mCameraLookAt.z = 0.0f;
    }

    void init(lite3dpp::Main *main) override
    {
        lite3dpp::Scene *scene = main->getResourceManager()->queryResource<lite3dpp::Scene>("SimpleScene",
            "samples:scenes/scene.json");
        mMinigun = scene->getObject("Minigun");
        mPlasmagun = scene->getObject("Plasmagun");

        lite3dpp::Camera *camera = scene->addCamera("MyCamera");
        camera->setupPerspective(1.0f, 1000.0f, 45.0f, 
            (float)main->window()->width() / (float)main->window()->height());

        camera->setPosition(mCameraPos);
        camera->lookAt(mCameraLookAt);

        main->window()->addCamera(camera, 1);
    }

    void shut(lite3dpp::Main *main) override
    {}

    void frameBegin(lite3dpp::Main *main) override
    {}

    void frameEnd(lite3dpp::Main *main) override
    {}

    void timerTick(lite3dpp::Main *main, lite3d_timer *timerid) override
    {
        mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.01f);
        mPlasmagun->getRoot()->rotateAngle(KM_VEC3_NEG_Z, 0.01f);
    }

    void processEvent(lite3dpp::Main *main, SDL_Event *e) override
    {
        if (e->type == SDL_KEYDOWN)
        {
            /* exit */
            if (e->key.keysym.sym == SDLK_ESCAPE)
                main->stop();
        }
    }

private:

    kmVec3 mCameraPos;
    kmVec3 mCameraLookAt;

    lite3dpp::SceneObject *mMinigun;
    lite3dpp::SceneObject *mPlasmagun;
};


int main(int agrc, char *args[])
{
    lite3dpp::Main mainObj;
    SampleLifecycleListener lifecycleListener;
    
    try
    {
        mainObj.registerLifecycleListener(&lifecycleListener);
        mainObj.initFromConfig("samples/config/config.json");
        mainObj.run();
    }
    catch(std::exception &ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
        return -1;
    }
    
    return 0;
}
