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

namespace lite3dpp {
namespace samples {

class CameraSample : public Sample
{
public:

    void createScene() override
    {
        // load empty scene
        auto scene = getMain().getResourceManager()->queryResource<Scene>("SampleCameraScene",
            "samples:scenes/empty.json");
        auto camera = getMain().getCamera("MyCamera");
        setMainCamera(camera);

        /* Добавляем пол */
        mGround = scene->addObject("Ground", "samples:objects/ground.json");
        /* Центральный кубик */
        mCenterBox = scene->addObject("CenterBox", "samples:objects/cube.json", mGround, kmVec3 {0.0f, 0.0f, 40.0f});
        /* Еще кубики */
        mBox1 = scene->addObject("box1", "samples:objects/cube.json", mGround, kmVec3 {100.0f, 400.0f, 40.0f});
        mBox2 = scene->addObject("box2", "samples:objects/cube.json", mCenterBox, kmVec3 {0.0f, 800.0f, 100.0f});
        mBox3 = scene->addObject("box3", "samples:objects/cube.json", mGround, kmVec3 {0.0f, 0.0f, 400.0f});
        /* По умолчанию привязвыем камеру к полу */
        scene->attachCamera(camera, mGround);
        mCameraCurrentBound = mGround;
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        mCenterBox->rotateZ(deltaRetard * 0.01);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_e)
            {
                auto next = mCameraCurrentBound == mGround ? mBox2 : mGround;
                /* перепривязка камеры к кубику */
                getMainCamera().rebase(next);
                mCameraCurrentBound = next;
            }
            else if (e->key.keysym.sym == SDLK_q)
            {
                /* перепривязка кубика к камере а потом к обратно к полу */
                if (mBox3->getParent() != &getMainCamera())
                {
                    mBox3->rebase(&getMainCamera());
                    mBox3->setPosition(kmVec3 { 0.0f, -100.0f, -400.0f});
                }
                else
                {
                    mBox3->rebase(mGround);
                    mBox3->setPosition(kmVec3 { 0.0f, 0.0f, 400.0f});
                }
            }
        }
    }
    
private:

    SceneObject *mCameraCurrentBound = nullptr;
    SceneObject *mGround = nullptr;
    SceneObject *mCenterBox = nullptr;
    SceneObject *mBox1 = nullptr;
    SceneObject *mBox2 = nullptr;
    SceneObject *mBox3 = nullptr;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::CameraSample sample;
    return sample.start("samples/config/config.json");
}
