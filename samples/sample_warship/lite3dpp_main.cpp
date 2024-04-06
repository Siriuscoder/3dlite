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

class DeferredShadingSample : public Sample
{
public:
    
    DeferredShadingSample() : 
        mGammaFactor(1.0f)
    {}

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("Warship",
            "warship:scenes/warship_prepass.json");

        setMainCamera(getMain().getCamera("MainCamera"));
        addFlashlight(scene);
        
        scene = getMain().getResourceManager()->queryResource<Scene>("WarshipCombine",
            "warship:scenes/warship_combine.json");
        scene = getMain().getResourceManager()->queryResource<Scene>("WarshipPostProcess",
            "warship:scenes/warship_postprocess.json");
        
        lite3dpp::Material::setIntGlobalParameter("FXAA", 0);

        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        lite3dpp::Material::setFloatv3GlobalParameter("screenResolution", resolution);
    }
    
    void addFlashlight(Scene *scene)
    {
        auto flashLightObject = scene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(0.0f);
        mFlashLight->getLight()->setAttenuationLinear(0.005f);
        mFlashLight->getLight()->setAttenuationQuadratic(0.000003f);
        mFlashLight->getLight()->setInfluenceDistance(1000.0f);
        mFlashLight->getLight()->setAngleInnerCone(0.70f);
        mFlashLight->getLight()->setAngleOuterCone(1.24f);
        mFlashLight->getLight()->setDiffuse(KM_VEC3_ONE);
        mFlashLight->getLight()->enabled(false);
    }

    void mainCameraChanged() override
    {
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        mFlashLight->setPosition(getMainCamera().getPosition());
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGammaFactor += 0.02;
                if (mGammaFactor > 2.2)
                    mGammaFactor = 2.2;
                lite3dpp::Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGammaFactor -= 0.02;
                if (mGammaFactor < 1.0)
                    mGammaFactor = 1.0;
                lite3dpp::Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_o)
            {
                static bool fxaaEnabled = false;
                fxaaEnabled = !fxaaEnabled;
                lite3dpp::Material::setIntGlobalParameter("FXAA", fxaaEnabled ? 1 : 0);
            }
            else if (e->key.keysym.sym == SDLK_l)
            {
                static bool flashLightEnabled = false;
                flashLightEnabled = !flashLightEnabled;
                mFlashLight->getLight()->enabled(flashLightEnabled);
            }
        }
        else if (e->type == SDL_MOUSEMOTION)
        {
            mFlashLight->setRotation(getMainCamera().getRotation());
        }
    }
    
private:
                    
    float mGammaFactor;
    LightSceneNode* mFlashLight;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::DeferredShadingSample sample;
    return sample.start("warship/config/warship_df.json");
}

