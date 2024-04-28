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
#include <lite3dpp_physics/lite3dpp_physics_scene.h>

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press 'c' to drop simple box\n"
    "Press 'e' to drop compound coss body\n"
    "Press 'x' to drop compound Z body\n"
    "Press 'q' to drop compound T body\n"
    "Press 'r' to show/hide coord arrows\n";

class BoxesColliderSample : public Sample
{
public:

    BoxesColliderSample() : 
        Sample(helpString)
    {}

    void createScene() override
    {
        // load empty scene with floor plane only
        mScene = getMain().getResourceManager()->queryResource<lite3dpp_phisics::PhysicsScene>("SamplePhysicsScene",
            "samples:scenes/physic_simple.json");
        setMainCamera(getMain().getCamera("MyCamera"));
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_c || 
                e->key.keysym.sym == SDLK_e || 
                e->key.keysym.sym == SDLK_x || 
                e->key.keysym.sym == SDLK_q)
            {
                if (mCubes.size() > 500)
                {
                    // delete oldest box
                    mScene->removeObject(mCubes.front()->getName() + "_arrows");
                    mScene->removeObject(mCubes.front()->getName());
                    mCubes.pop_front();
                }

                String cubeName("Cube");
                cubeName.append(std::to_string(mBoxCounter++));

                kmVec3 pos = {
                    static_cast<float>(rand() % 1000),
                    static_cast<float>(rand() % 1000),
                    static_cast<float>((rand() % 1000) + 1000)
                };

                kmQuaternion rot = { 1.0f, 1.0f, 1.0f, (rand() % 1000)/1000.0f };
                auto newObject = mScene->addObject(cubeName, 
                    e->key.keysym.sym == SDLK_c ? "samples:objects/cube.json" : 
                    (e->key.keysym.sym == SDLK_e ? "samples:objects/compound_cross.json" : 
                    (e->key.keysym.sym == SDLK_x ? "samples:objects/compound_Z.json" : 
                    "samples:objects/compound_T.json")),
                    nullptr, pos, rot);

                auto arrowObject = mScene->addObject(cubeName + "_arrows", "samples:objects/coord_arrows.json",
                    newObject, KM_VEC3_ZERO, KM_QUATERNION_IDENTITY, kmVec3 { 2.4, 2.4, 2.4 });
                mArrowsEnabled ? arrowObject->enable() : arrowObject->disable();
                
                mCubes.push_back(newObject);
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                mArrowsEnabled = !mArrowsEnabled;
                for (auto &o : mScene->getObjects())
                {
                    if (o.first.find("_arrows") != std::string::npos)
                    {
                        mArrowsEnabled ? o.second->enable() : o.second->disable();
                    }
                }
            }
        }
    }
    
private:

    stl<SceneObject *>::list mCubes;
    Scene *mScene = nullptr;
    int mBoxCounter = 0;
    bool mArrowsEnabled = true;

};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::BoxesColliderSample sample;
    return sample.start("samples/config/config.json");
}
