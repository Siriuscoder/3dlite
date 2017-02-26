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
        setMainCamera(scene->getCamera("MainCamera"));
        
        scene = getMain().getResourceManager()->queryResource<Scene>("WarshipCombine",
            "warship:scenes/warship_combine.json");
        scene = getMain().getResourceManager()->queryResource<Scene>("WarshipPostProcess",
            "warship:scenes/warship_postprocess.json");
        
        lite3dpp::Material::setIntGlobalParameter("FXAA", 0);
        lite3dpp::Material::setIntGlobalParameter("Gamma", 1);
        lite3dpp::Material::setIntGlobalParameter("glowEnabled", 1);
    }
    
    void timerTick(lite3d_timer *timerid) override
    {
        Sample::timerTick(timerid);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
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
        }
    }
    
private:
                    
    float mGammaFactor;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::DeferredShadingSample sample;
    return sample.start("warship/config/warship_df.json");
}
