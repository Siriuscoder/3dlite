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

class SponzaHall : public Sample, public RenderTargetObserver
{
public:
    const kmVec3 sunLightDirection = { 2.0f, -1.0f, -2.3f };

    void createScene() override
    {
        mMainScene = getMain().getResourceManager()->queryResource<Scene>("Sponza",
            "sponzamat:scenes/sponza.json");
        getMain().getResourceManager()->queryResource<Scene>("SkyScene",
            "sponzamat:scenes/sky.json");
        getMain().getResourceManager()->queryResource<Scene>("PostprocessScene",
            "sponzamat:scenes/postprocess.json");

        setMainCamera(getMain().getCamera("MyCamera"));

        getMain().window()->depthTestFunc(RenderTarget::TestFuncLEqual);
        // get shadowmap render target, it was been loaded with Sponza scene
        mShadowMap = getMain().getResourceManager()->queryResource<TextureRenderTarget>("RenderShadowMap");
        mShadowMap->addObserver(this);

        Camera *shadowView = getMain().getCamera("ShadowCamera");
        shadowView->setDirection(sunLightDirection);
        shadowView->setCullFaceMode(Camera::CullFaceFront);
        

        lite3dpp::Material::setFloatm4GlobalParameter("shadowMatrix", shadowView->getProjTransformMatrix());
        lite3dpp::Material::setIntGlobalParameter("FXAA", 1);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());

        addSunlight();

        getMain().getResourceManager()->releaseFileCache();
    }

    void mainCameraChanged() override
    {
        Sample::mainCameraChanged();
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

    void addSunlight()
    {
        String lightParams = ConfigurationWriter().set(L"Name", "SunLight.node").set(L"Light",
            ConfigurationWriter().set(L"Ambient", KM_VEC3_ZERO)
            .set(L"Diffuse", KM_VEC3_ONE)
            .set(L"Position", KM_VEC3_ZERO)
            .set(L"Name", "SunLight")
            .set(L"Specular", KM_VEC3_ONE)
            .set(L"SpotDirection", sunLightDirection)
            .set(L"Type", "Directional")).write();

        mSunLight.reset(new LightSceneNode(ConfigurationReader(lightParams.data(), lightParams.size()), NULL, &getMain()));
        mSunLight->addToScene(mMainScene);
        mSunLight->getLight()->enabled(true);
        mSunLight->frustumTest(false);
    }

    void postUpdate(RenderTarget *rt) override
    {
        if (rt == mShadowMap)
        {
            // render shadow map at once
            rt->disable();
        }
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_o)
            {
                static bool fxaaEnabled = true;
                fxaaEnabled = !fxaaEnabled;
                lite3dpp::Material::setIntGlobalParameter("FXAA", fxaaEnabled ? 1 : 0);
            }
        }
    }

private:

    std::unique_ptr<LightSceneNode> mSunLight;
    Scene *mMainScene;
    RenderTarget *mShadowMap;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SponzaHall sample;
    return sample.start("sponza/config/config_sponza.json");
}
