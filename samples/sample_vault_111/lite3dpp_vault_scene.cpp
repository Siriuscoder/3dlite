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
#include "lite3dpp_vault_bloom.h"

namespace lite3dpp {
namespace samples {

class SampleVault111 : public Sample
{
public:

    struct SpotLightWithShadow
    {
        SceneNode* spot = nullptr;
        SampleShadowManager::ShadowCaster* shadowCaster = nullptr;

        void rotateAngle(const kmVec3 &axis, float angle)
        {
            spot->rotateAngle(axis, angle);
            shadowCaster->invalidate();
        }
    };

public:

    void createScene() override
    {
        mShadowManager = std::make_unique<SampleShadowManager>(getMain());
        mBloomEffectRenderer = std::make_unique<SampleBloomEffect>(getMain());
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111", "vault_111:scenes/vault_111.json");
        setMainCamera(getMain().getCamera("MyCamera"));

        setupShadowCasters();
        addFlashlight();
        // load intermediate light compute scene
        mCombineScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111_LightCompute",
            "vault_111:scenes/lightpass.json");

        // Load bloom effect pipeline
        mBloomEffectRenderer->init();

        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("Vault_111_Postprocess",
            "vault_111:scenes/postprocess.json");
        
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);

        kmVec3 resolution = { 
            static_cast<float>(getMain().window()->width()), 
            static_cast<float>(getMain().window()->height()), 0 
        };
        Material::setFloatv3GlobalParameter("screenResolution", resolution);

        mMinigun01 = mVaultScene->getObject("MinigunTurret");
        mMinigun02 = mVaultScene->getObject("MinigunTurret.001");
        mMinigun02->getNode("Minigun")->rotateAngle(KM_VEC3_POS_Z, kmDegreesToRadians(-30.0));
        mGearKey = mVaultScene->getObject("VaultStatic")->getNode("GearKey");
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.001"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.002"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.003"));
    }

    void setupShadowCasters()
    {
        RenderTarget* shadowUpdateRT = getMain().getResourceManager()->queryResource<TextureRenderTarget>("ShadowPass");
        shadowUpdateRT->addObserver(mShadowManager.get());
        mVaultScene->addObserver(mShadowManager.get());

        // Установим тень для трех прожекторов и потом будем их вращать
        // Источники света получаем по ObjectName + NodeName
        mSpot01 = SpotLightWithShadow {
            mVaultScene->getObject("LightSpot")->getNode("LightSpotLamp"),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpotLightSpotNode"))
        };

        mSpot02 = SpotLightWithShadow {
            mVaultScene->getObject("LightSpot.002")->getNode("LightSpotLamp"),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpot.002LightSpotNode"))
        };

        mSpot03 = SpotLightWithShadow {
            mVaultScene->getObject("LightSpot.003")->getNode("LightSpotLamp"),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpot.003LightSpotNode"))
        };
        
        mShadowManager->newShadowCaster(mVaultScene->getLightNode("VaultStaticRotorSpot"));
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

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        float animPiNew = mAnimPi + (0.05f * deltaRetard);
        mAnimPi = animPiNew > (2.0 * M_PI) ? animPiNew - (2 * M_PI) : animPiNew;

        float cosA = cos(mAnimPi);
        mMinigun01->getNode("Minigun")->rotateAngle(KM_VEC3_POS_Z, cosA * 0.02);
        mMinigun01->getNode("MinigunBarrel")->rotateAngle(KM_VEC3_POS_Y, 0.13 * deltaRetard);
        mMinigun02->getNode("Minigun")->rotateAngle(KM_VEC3_POS_Z, -cosA * 0.02);
        mMinigun02->getNode("MinigunBarrel")->rotateAngle(KM_VEC3_POS_Y, 0.13 * deltaRetard);
        mSpot03.rotateAngle(KM_VEC3_POS_Z, 0.1 * deltaRetard);

        std::for_each(mFans.begin(), mFans.end(), [deltaRetard](SceneNode *fanRotor)
        {
            fanRotor->rotateAngle(KM_VEC3_POS_Z, 0.07 * deltaRetard);
        });

        mVaultScene->getObject("VaultStatic")->getNode("GearKeySpinner")->rotateAngle(KM_VEC3_POS_X, 0.15 * deltaRetard);
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
            else if (e->key.keysym.sym == SDLK_p)
            {
                mSpot01.rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.keysym.mod & KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.keysym.sym == SDLK_o)
            {
                mSpot02.rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.keysym.mod & KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.keysym.sym == SDLK_k)
            {
                mGearKey->move(kmVec3{10.0f * (e->key.keysym.mod & KMOD_LCTRL ? -1.0f : 1.0f), 0.0f, 0.0f});
                auto pos = mGearKey->getPosition();
                if (pos.x > 332.0f)
                {
                    pos.x = 332.0f;
                    mGearKey->setPosition(pos);
                }
                else if (pos.x < -58.0f)
                {
                    pos.x = -58.0f;
                    mGearKey->setPosition(pos);
                }
            }
        }
    }

private:

    Scene* mCombineScene = nullptr;
    Scene* mVaultScene = nullptr;
    std::unique_ptr<SampleShadowManager> mShadowManager;
    std::unique_ptr<SampleBloomEffect> mBloomEffectRenderer;
    std::unique_ptr<LightSceneNode> mFlashLight;
    SpotLightWithShadow mSpot01;
    SpotLightWithShadow mSpot02;
    SpotLightWithShadow mSpot03;
    SceneNode* mGearKey = nullptr;
    stl<SceneNode *>::vector mFans;
    SceneObject* mMinigun01 = nullptr;
    SceneObject* mMinigun02 = nullptr; 
    float mAnimPi = 0.0f;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

