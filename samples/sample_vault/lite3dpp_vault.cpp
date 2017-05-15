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
#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class Vault : public Sample
{
public:
    
    Vault() : 
        mGammaFactor(1.0f)
    {}

    void createScene() override
    {
        // load main scene as precompute step
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("Vault",
            "vaultmat:scenes/prepass.json");
        setMainCamera(scene->getCamera("MyCamera"));
        // load intermediate light compute scene and setup lighting 
        setupLightPassScene(scene, getMain().getResourceManager()->queryResource<Scene>("VaultLightComputeStep",
            "vaultmat:scenes/lightpass.json"));
        // Scene that combines lightmap from previous step and textures and draw all transparent objects at end of step.
        getMain().getResourceManager()->queryResource<Scene>("VaultCombineStep",
            "vaultmat:scenes/combine.json");
        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("VaultPostprocessStep",
            "vaultmat:scenes/postprocess.json");
    }
    
    void setupLightPassScene(Scene *prepass, Scene *scene)
    {
        for (const auto &light : prepass->getLights())
        {
            /* load per light big triangle */
            SceneObject *lo = scene->addObject(light.first, "vault:objects/lightpass_tri.json", NULL);
            /* load per light big triangle material and setup light properties as uniform parameters */
            Material *material = getMain().getResourceManager()->queryResource<Material>(light.first + ".material",
                "vaultmat:materials/lightpass.json");

            MeshSceneNode *mnode = (MeshSceneNode *)lo->getNode("Root");
            SDL_assert(mnode);


            LightSource lsw(light.second->lightSourceToWorld(), NULL);
            material->setIntParameter(0, "light.enabled", lsw.enabled() ? 1 : 0, false);
            material->setIntParameter(0, "light.type", lsw.getType(), false);
            material->setFloatParameter(0, "light.spotangle", lsw.getSpotFactor().x, false);
            material->setFloatv3Parameter(0, "light.position", lsw.getPosition(), false);
            material->setFloatv3Parameter(0, "light.diffuse", lsw.getDiffuse(), false);
            material->setFloatv3Parameter(0, "light.ambient", lsw.getAmbient(), false);
            material->setFloatv3Parameter(0, "light.specular", lsw.getSpecular(), false);
            material->setFloatv3Parameter(0, "light.direction", lsw.getSpotDirection(), false);
            material->setFloatv4Parameter(0, "light.attenuation", lsw.getAttenuation(), false);

            mnode->frustumTest(false);
            mnode->setName(light.first);
            mnode->replaceMaterial(0, material);
        }
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
            else if (e->key.keysym.sym == SDLK_o)
            {
                static bool fxaaEnabled = false;
                fxaaEnabled = !fxaaEnabled;
                lite3dpp::Material::setIntGlobalParameter("FXAA", fxaaEnabled ? 1 : 0);
            }
        }
    }
    
private:
    
    float mGammaFactor;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::Vault sample;
    return sample.start("vault/config/config_vault.json");
}

