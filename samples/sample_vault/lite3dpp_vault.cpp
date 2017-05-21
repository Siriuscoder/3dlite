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

class Vault : public Sample, public SceneObserver
{
public:
    
    Vault() : 
        mGammaFactor(1.0f),
        mVaultScene(NULL),
        mLightComputeStep(NULL),
        mAnimCounter(0)
    {}

    void createScene() override
    {
        // load main scene as precompute step
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault",
            "vaultmat:scenes/prepass.json");
        setMainCamera(mVaultScene->getCamera("MainCamera"));
        // load intermediate light compute scene
        getMain().getResourceManager()->queryResource<Scene>("VaultLightComputeStep",
            "vaultmat:scenes/lightpass.json")->addObserver(this);
        mLightComputeStep = getMain().getResourceManager()->queryResource<TextureRenderTarget>("LightComputeStep");
        // Scene that combines lightmap from previous step and textures and draw all transparent objects at end of step.
        getMain().getResourceManager()->queryResource<Scene>("VaultCombineStep",
            "vaultmat:scenes/combine.json");
        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("VaultPostprocessStep",
            "vaultmat:scenes/postprocess.json");
        
        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        lite3dpp::Material::setFloatv3GlobalParameter("screenResolution", resolution);
        lite3dpp::Material::setIntGlobalParameter("FXAA", 1);
    }

    // setup lighting at once before light compute scene begin rendering first time
    void beginSceneRender(Scene *scene, Camera *camera) override
    {
        SDL_assert(mVaultScene);
        /* check scene already fullup */
        if (scene->getObjects().size() > 0)
            return;

        setupLightPassScene(mVaultScene, scene);
    }

    void endSceneRender(Scene *scene, Camera *camera) override 
    {
        /* disable lightpass after lightmap recalc complete */
        SDL_assert(mLightComputeStep);
        mLightComputeStep->disable();
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

            MeshSceneNode *mnode = (MeshSceneNode *)lo->getRoot();
            SDL_assert(mnode);


            LightSource lsw(light.second->lightSourceToWorld(), NULL);
            material->setIntParameter(1, "light.enabled", lsw.enabled() ? 1 : 0, false);
            material->setIntParameter(1, "light.type", lsw.getType(), false);
            material->setFloatv3Parameter(1, "light.spotfactor", lsw.getSpotFactor(), false);
            material->setFloatv3Parameter(1, "light.position", lsw.getPosition(), false);
            material->setFloatv3Parameter(1, "light.diffuse", lsw.getDiffuse(), false);
            material->setFloatv3Parameter(1, "light.ambient", lsw.getAmbient(), false);
            material->setFloatv3Parameter(1, "light.specular", lsw.getSpecular(), false);
            material->setFloatv3Parameter(1, "light.direction", lsw.getSpotDirection(), false);
            material->setFloatv4Parameter(1, "light.attenuation", lsw.getAttenuation(), false);

            mnode->frustumTest(false);
            mnode->setName(light.first);
            mnode->replaceMaterial(0, material);
        }
    }
    
    void timerTick(lite3d_timer *timerid) override
    {
        Sample::timerTick(timerid);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        
        if (timerid == getMain().getFixedUpdateTimer())
        {
            mAnimCounter = mAnimCounter >= 1.0f ? 0.0f : mAnimCounter + 0.005f;           
            lite3dpp::Material::setFloatGlobalParameter("animcounter", mAnimCounter);
        }
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
                static bool fxaaEnabled = true;
                fxaaEnabled = !fxaaEnabled;
                lite3dpp::Material::setIntGlobalParameter("FXAA", fxaaEnabled ? 1 : 0);
            }
        }
    }

    /* enable lightpass then the main camera state changed to recalc lightmap */
    void mainCameraChanged() override
    {
        SDL_assert(mLightComputeStep);
        mLightComputeStep->enable();
    }
    
private:
    
    float mGammaFactor;
    Scene *mVaultScene;
    RenderTarget *mLightComputeStep;
    float mAnimCounter;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::Vault sample;
    return sample.start("vault/config/config_vault.json");
}

