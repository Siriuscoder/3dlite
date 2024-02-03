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

namespace lite3dpp {
namespace samples {

class RenderToTexture : public Sample
{
public:

    RenderToTexture() : 
        mMinigun(NULL),
        mPlasmagun(NULL)
    {}

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("SceneInTexture",
            "samples:scenes/scene_rtt.json");
        mMinigun = scene->getObject("Minigun");
        mPlasmagun = scene->getObject("Plasmagun");

        scene = getMain().getResourceManager()->queryResource<Scene>("BoxScene",
            "samples:scenes/scene_rtt_box.json");
        setMainCamera(getMain().getCamera("MyCamera"));

        mBox = scene->getObject("Box");
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        const float deltaRotate = 0.01f * deltaRetard;
        mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, deltaRotate);
        mPlasmagun->getRoot()->rotateAngle(KM_VEC3_NEG_Z, deltaRotate);
        mBox->getRoot()->rotateAngle(KM_VEC3_NEG_Z, deltaRotate);
    }

private:

    SceneObject *mMinigun;
    SceneObject *mPlasmagun;
    SceneObject *mBox;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::RenderToTexture sample;
    return sample.start("samples/config/config.json");
}
