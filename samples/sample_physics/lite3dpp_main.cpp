/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2017  Sirius (Korolev Nikita)
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

#include "lite3dpp_base.h"

namespace lite3dpp {
namespace samples {

class BoxesColliderSample : public PhysicSampleBase
{
public:

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_c)
            {
                if (mCubes.size() > 500)
                {
                    // delete oldest boxc
                    mCubes.pop_front();
                }

                String cubeName("Cube");
                cubeName.append(std::to_string(mBoxCounter++));

                BaseBody::Ptr box = createBox(cubeName);
                mCubes.push_back(box);

                kmVec3 pos = {
                    static_cast<float>(rand() % 1000),
                    static_cast<float>(rand() % 1000),
                    static_cast<float>((rand() % 1000) + 1000)
                };

                kmQuaternion rot = { 1.0f, 1.0f, 1.0f, (rand() % 1000)/1000.0f };
                box->setPosition(pos);
                box->setRotation(rot);
            }
        }
    }

    // been called after exit from render loop, before release any resources
    void shut() override
    {
        PhysicSampleBase::shut();
        // delete all objects
        mCubes.clear();
    }
    
private:

    stl<BaseBody::Ptr>::list mCubes;
    int mBoxCounter = 0;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::BoxesColliderSample sample;
    return sample.start("samples/config/config.json");
}
