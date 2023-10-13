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
#include "lite3dpp_vault_shadows.h"

namespace lite3dpp {
namespace samples {

class SampleVault111 : public Sample
{
public:

    void createScene() override
    {
        mShadowManager = std::make_unique<SampleShadowManager>(getMain());
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111", "vault_111:scenes/vault_111.json");
        setMainCamera(getMain().getCamera("MyCamera"));

        setupShadowCasters();
        addFlashlight();
        // load intermediate light compute scene
        mCombineScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111_LightCompute",
            "vault_111:scenes/lightpass.json");

        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("Vault_111_Postprocess",
            "vault_111:scenes/postprocess.json");
        
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);

        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        Material::setFloatv3GlobalParameter("screenResolution", resolution);
    }

    void setupShadowCasters()
    {
        RenderTarget* shadowUpdateRT = getMain().getResourceManager()->queryResource<TextureRenderTarget>("ShadowPass");
        shadowUpdateRT->addObserver(mShadowManager.get());

        auto staticScene = mVaultScene->getObject("VaultStatic");
        // Установим тень для трех прожекторов и потом будем их вращать
        mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpotNode"));
        mSpot01 = staticScene->getNode("LightSpot");

        mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpotNode.001"));
        mSpot02 = staticScene->getNode("LightSpot.001");

        mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpotNode.002"));
        mSpot03 = staticScene->getNode("LightSpot.002");
    }

    void addFlashlight()
    {
        ConfigurationWriter flashlightJson;
        LightSource flashlight("FlashLight");
        flashlight.setAttenuationConstant(0.0f);
        flashlight.setAttenuationLinear(50.0f);
        flashlight.setAttenuationQuadratic(130.0f);
        flashlight.setAngleInnerCone(0.80f);
        flashlight.setAngleOuterCone(1.00f);
        flashlight.setDiffuse(kmVec3 {1.0f, 233.0f / 255.0f, 173.0f / 255.0f });
        flashlight.setDirection(KM_VEC3_NEG_Z);
        flashlight.setPosition(KM_VEC3_ZERO);
        flashlight.setType(LITE3D_LIGHT_SPOT);
        flashlight.setRadiance(800000.0f);
        flashlight.setInfluenceMinRadiance(0.001f);
        flashlight.toJson(flashlightJson);

        String flashLightParams = ConfigurationWriter().set(L"Name", "FlashLight.node").set(L"Light", flashlightJson).write();
        mFlashLight.reset(new LightSceneNode(ConfigurationReader(flashLightParams.data(), flashLightParams.size()), NULL, &getMain()));
        mFlashLight->addToScene(mVaultScene);
        mFlashLight->getLight()->enabled(false);
        mFlashLight->frustumTest(false);
    }

    void mainCameraChanged() override
    {
        Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
        updateFlashLight();
    }

    void updateFlashLight()
    {
        if (mFlashLight && mFlashLight->getLight()->enabled())
        {
            mFlashLight->setPosition(getMainCamera().getPosition());
            mFlashLight->setRotation(getMainCamera().getRotation());
        }
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_l && mFlashLight)
            {
                static bool flashLightEnabled = false;
                flashLightEnabled = !flashLightEnabled;
                mFlashLight->getLight()->enabled(flashLightEnabled);
                updateFlashLight();
            }
        }
    }

private:

    Scene* mCombineScene = nullptr;
    Scene* mVaultScene = nullptr;
    std::unique_ptr<SampleShadowManager> mShadowManager;
    std::unique_ptr<LightSceneNode> mFlashLight;
    SceneNode* mSpot01 = nullptr;
    SceneNode* mSpot02 = nullptr;
    SceneNode* mSpot03 = nullptr;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

