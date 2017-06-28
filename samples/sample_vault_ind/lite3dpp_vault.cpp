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
        mGammaFactor(1.0f),
        mVaultScene(NULL),
        mAnimCounter(0),
        mAnimPi(0)
    {}

    void createScene() override
    {
        // load main scene, direct render depth and then calculate lighting
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault",
            "vaultmat:scenes/directrender.json");
        setMainCamera(mVaultScene->getCamera("MainCamera"));
        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("VaultPostprocessStep",
            "vaultmat:scenes/postprocess.json");
        
        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        lite3dpp::Material::setFloatv3GlobalParameter("screenResolution", resolution);
        lite3dpp::Material::setIntGlobalParameter("FXAA", 1);
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        getMain().window()->depthTestFunc(LITE3D_TEST_LEQUAL);
        // use instancing by default
        mVaultScene->instancingMode(true);

        mReactLamp01 = mVaultScene->getLightNode("ReactorLamp_01.node");
        mReactLamp02 = mVaultScene->getLightNode("ReactorLamp_02.node");
        mMinigun = mVaultScene->getObject("Minigun");
        mGatling = mVaultScene->getObject("Gatling");
        mLazer = mVaultScene->getObject("Lazer");
    }
    
    void timerTick(lite3d_timer *timerid) override
    {
        Sample::timerTick(timerid);
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        
        if (timerid == getMain().getFixedUpdateTimer())
        {
            mAnimCounter = mAnimCounter >= 1.0f ? 0.0f : mAnimCounter + 0.005f;
            mAnimPi = mAnimPi >= 2 * M_PI ? 0.0f : mAnimPi + 0.23f;
            
            lite3dpp::Material::setFloatGlobalParameter("animcounter", mAnimCounter);

            kmVec4 attenuation = mReactLamp01->getLight()->getAttenuation();
            attenuation.x = (cos(mAnimPi) + 1) / 2;
            attenuation.y = attenuation.x / 100;
            mReactLamp01->getLight()->setAttenuation(attenuation);
            mReactLamp02->getLight()->setAttenuation(attenuation);
            
            float soarDelta = cos(mAnimPi) * 5;
            mGatling->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mGatling->getRoot()->setPosZ(-225 + soarDelta);
            
            mMinigun->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mMinigun->getRoot()->setPosZ(-225 + soarDelta);
            mMinigun->getNode("MinigunBurrel.node")->rotateAngle(KM_VEC3_POS_X, 0.23f);
            
            mLazer->getRoot()->rotateAngle(KM_VEC3_POS_Z, 0.05f);
            mLazer->getRoot()->setPosZ(-225 + soarDelta);
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
    
private:
    
    float mGammaFactor;
    Scene *mVaultScene;
    float mAnimCounter;
    float mAnimPi;
    LightSceneNode *mReactLamp01;
    LightSceneNode *mReactLamp02;
    SceneObject *mMinigun;
    SceneObject *mGatling;
    SceneObject *mLazer;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::Vault sample;
    return sample.start("vault_1/config/config_vault.json");
}

