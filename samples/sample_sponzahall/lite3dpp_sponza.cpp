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

static const char *helpString = 
    "Press 'e' to show shadow map\n";

class SponzaHall : public Sample, public RenderTargetObserver
{
public:
    const kmVec3 sunLightDirection = { -2.0f, -1.0f, -2.3f };

    SponzaHall() : 
        Sample(helpString)
    {}

    void createScene() override
    {
        mMainScene = getMain().getResourceManager()->queryResource<Scene>("Sponza",
            "sponzamat:scenes/sponza.json");
        getMain().getResourceManager()->queryResource<Scene>("SkyScene",
            "sponzamat:scenes/sky.json");
        getMain().getResourceManager()->queryResource<Scene>("PostprocessScene",
            "sponzamat:scenes/postprocess.json");

        setMainCamera(getMain().getCamera("MyCamera"));

        RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
        // get shadowmap render target, it was been loaded with Sponza scene
        mShadowMap = getMain().getResourceManager()->queryResource<TextureRenderTarget>("RenderShadowMap");
        mShadowMap->addObserver(this);

        Camera *shadowView = getMain().getCamera("ShadowCamera");
        shadowView->setDirection(sunLightDirection);
        shadowView->setCullFaceMode(Camera::CullFaceFront);

        // Привязываем главную камеру к обьекту SponzaHall чтобы она крутилась вместе с ним 
        mSponzahall = mMainScene->getObject("SponzaHall");
        mMainScene->attachCamera(&getMainCamera(), mSponzahall);

        lite3dpp::Material::setFloatm4GlobalParameter("shadowMatrix", shadowView->refreshProjViewMatrix());
        lite3dpp::Material::setIntGlobalParameter("SwitchView", 0);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getWorldPosition());

        addSunlight();

        getMain().getResourceManager()->releaseFileCache();
    }

    void mainCameraChanged() override
    {
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getWorldPosition());
    }

    void addSunlight()
    {
        auto sunlightObject = mMainScene->addObject("SunLight", "samples:objects/sunlight.json");
        mSunLight = sunlightObject->getLightNode("SunLight.node");
        mSunLight->getLight()->setDirection(sunLightDirection);
    }

    void postUpdate(RenderTarget *rt) override
    {
        if (rt == mShadowMap)
        {
            // render shadow map at once
            rt->disable();
        }
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        // Имитируем вращение неба и солнца, но на самом деле крутится только здание
        mSponzahall->rotateZ(0.002f * deltaRetard);
        // Перересуем тень после поворота, в след кадре
        mShadowMap->enable();
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_e)
            {
                static bool SwitchView = false;
                SwitchView = !SwitchView;
                lite3dpp::Material::setIntGlobalParameter("SwitchView", SwitchView ? 1 : 0);
            }
        }
    }

private:

    LightSceneNode *mSunLight;
    Scene *mMainScene;
    SceneObject *mSponzahall;
    RenderTarget *mShadowMap;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SponzaHall sample;
    return sample.start("sponza/config/config_sponza.json");
}
