/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
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

class SponzaHall : public Sample
{
public:

    void createScene() override
    {
        mMainScene = getMain().getResourceManager()->queryResource<Scene>("Sponza",
            "sponzamat:scenes/sponza.json");
        getMain().getResourceManager()->queryResource<Scene>("SkyScene",
            "sponzamat:scenes/sky.json");
        getMain().getResourceManager()->queryResource<Scene>("PostprocessScene",
            "sponzamat:scenes/postprocess.json");

        setMainCamera(getMain().getCamera("MyCamera"));

        getMain().window()->depthTestFunc(LITE3D_TEST_LEQUAL);

        lite3dpp::Material::setIntGlobalParameter("FXAA", 1);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());

        addSunlight();
    }

    void mainCameraChanged() override
    {
        Sample::mainCameraChanged();
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

    void addSunlight()
    {
        kmVec3 spotDirection = { 2.0f, -1.0f, -2.3f };
        String lightParams = ConfigurationWriter().set(L"Name", "SunLight.node").set(L"Light",
            ConfigurationWriter().set(L"Ambient", KM_VEC3_ZERO)
            .set(L"Diffuse", KM_VEC3_ONE)
            .set(L"Position", KM_VEC3_ZERO)
            .set(L"Name", "SunLight")
            .set(L"Specular", KM_VEC3_ONE)
            .set(L"SpotDirection", spotDirection)
            .set(L"Type", "Directional")).write(true);

        mSunLight.reset(new LightSceneNode(ConfigurationReader(lightParams.data(), lightParams.size()), NULL, &getMain()));
        mSunLight->addToScene(mMainScene);
        mSunLight->getLight()->enabled(true);
        mSunLight->frustumTest(false);
    }

private:

    std::unique_ptr<LightSceneNode> mSunLight;
    Scene *mMainScene;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SponzaHall sample;
    return sample.start("sponza/config/config_sponza.json");
}
