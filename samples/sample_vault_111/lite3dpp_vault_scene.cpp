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
#include "lite3dpp_vault_shadows.h"
#include "lite3dpp_vault_bloom.h"

namespace lite3dpp {
namespace samples {

class SampleVault111 : public Sample
{
public:

    struct SpotLightWithShadow
    {
        SampleShadowManager::DynamicNode* spot = nullptr;
        SampleShadowManager::ShadowCaster* shadowCaster = nullptr;

        void rotateAngle(const kmVec3 &axis, float angle)
        {
            SDL_assert(spot);
            SDL_assert(shadowCaster);

            spot->rotateAngle(axis, angle);
            shadowCaster->invalidate();
        }
    };

    class MinigunObject
    {
    public:

        MinigunObject() = default;

        MinigunObject(Scene* scene, SampleShadowManager* shadowManager, String name) : 
            mMinigunObj(scene->getObject(name))
        {
            mMinigun = shadowManager->registerDynamicNode(mMinigunObj->getNode("Minigun"));
            mMinigunBarrel = shadowManager->registerDynamicNode(mMinigunObj->getNode("MinigunBarrel"));
        }

        void rotateAngle(const kmVec3 &axis, float angle)
        {
            SDL_assert(mMinigun);
            mMinigun->rotateAngle(axis, angle);
        }

        void animate(float angleMinigun, float angleBarrel)
        {
            mMinigun->rotateAngle(KM_VEC3_POS_Z, angleMinigun);
            mMinigunBarrel->rotateAngle(KM_VEC3_POS_Y, angleBarrel);
        }

    private:

        SceneObject* mMinigunObj = nullptr;
        SampleShadowManager::DynamicNode* mMinigun = nullptr;
        SampleShadowManager::DynamicNode* mMinigunBarrel = nullptr;
    };

public:

    void createScene() override
    {
        mShadowManager = std::make_unique<SampleShadowManager>(getMain());
        mBloomEffectRenderer = std::make_unique<SampleBloomEffect>(getMain());
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111", "vault_111:scenes/vault_111.json");
        getMain().getResourceManager()->queryResource<Scene>("ShadowClean", "vault_111:scenes/shadow_clean.json");
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

        mMinigun01 = MinigunObject(mVaultScene, mShadowManager.get(), "MinigunTurret");
        mMinigun02 = MinigunObject(mVaultScene, mShadowManager.get(), "MinigunTurret.001");
        mMinigun02.rotateAngle(KM_VEC3_POS_Z, kmDegreesToRadians(-30.0));

        mGearKey = mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("GearKey"));
        mGearKeySpinner = mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("GearKeySpinner"));
        mGeneratorSpinner01 = mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("PowerGeneratorSpinner01"));
        mGeneratorSpinner02 = mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("PowerGeneratorSpinner02"));
        mFans.emplace_back(mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("FanRotor")));
        mFans.emplace_back(mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.001")));
        mFans.emplace_back(mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.002")));
        mFans.emplace_back(mShadowManager->registerDynamicNode(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.003")));
    }

    void setupShadowCasters()
    {
        RenderTarget* shadowUpdateRT = getMain().getResourceManager()->queryResource<TextureRenderTarget>("ShadowPass");
        shadowUpdateRT->addObserver(mShadowManager.get());
        mVaultScene->addObserver(mShadowManager.get());

        // Установим тень для трех прожекторов и потом будем их вращать
        // Источники света получаем по ObjectName + NodeName
        mSpot = SpotLightWithShadow {
            mShadowManager->registerDynamicNode(mVaultScene->getObject("LightSpot")->getNode("LightSpotLamp")),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpotLightSpotNode"))
        };

        mSpot01 = SpotLightWithShadow {
            mShadowManager->registerDynamicNode(mVaultScene->getObject("LightSpot.001")->getNode("LightSpotLamp")),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpot.001LightSpotNode"))
        };

        mSpot02 = SpotLightWithShadow {
            mShadowManager->registerDynamicNode(mVaultScene->getObject("LightSpot.002")->getNode("LightSpotLamp")),
            mShadowManager->newShadowCaster(mVaultScene->getLightNode("LightSpot.002LightSpotNode"))
        };

        mSpot03 = SpotLightWithShadow {
            mShadowManager->registerDynamicNode(mVaultScene->getObject("LightSpot.003")->getNode("LightSpotLamp")),
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
        mMinigun01.animate(cosA * 0.02, 0.13 * deltaRetard);
        mMinigun02.animate(-cosA * 0.02, 0.13 * deltaRetard);
        mSpot03.rotateAngle(KM_VEC3_POS_Z, 0.1 * deltaRetard);

        std::for_each(mFans.begin(), mFans.end(), [deltaRetard](SampleShadowManager::DynamicNode* fanRotor)
        {
            fanRotor->rotateAngle(KM_VEC3_POS_Z, 0.07 * deltaRetard);
        });

        mGearKeySpinner->rotateAngle(KM_VEC3_POS_X, 0.1 * deltaRetard);
        mGeneratorSpinner01->rotateAngle(KM_VEC3_POS_Y, 0.025 * deltaRetard);
        mGeneratorSpinner02->rotateAngle(KM_VEC3_POS_Y, -0.025 * deltaRetard);
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
                mSpot.rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.keysym.mod & KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.keysym.sym == SDLK_i)
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
            else if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGammaFactor += 0.02;
                if (mGammaFactor > 3.0)
                    mGammaFactor = 3.0;
                Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGammaFactor -= 0.02;
                if (mGammaFactor < 1.5)
                    mGammaFactor = 1.5;
                Material::setFloatGlobalParameter("GammaFactor", mGammaFactor);
            }
        }
    }

private:

    Scene* mCombineScene = nullptr;
    Scene* mVaultScene = nullptr;
    std::unique_ptr<SampleShadowManager> mShadowManager;
    std::unique_ptr<SampleBloomEffect> mBloomEffectRenderer;
    std::unique_ptr<LightSceneNode> mFlashLight;
    SpotLightWithShadow mSpot;
    SpotLightWithShadow mSpot01;
    SpotLightWithShadow mSpot02;
    SpotLightWithShadow mSpot03;
    SampleShadowManager::DynamicNode* mGearKey = nullptr;
    SampleShadowManager::DynamicNode* mGearKeySpinner = nullptr;
    SampleShadowManager::DynamicNode* mGeneratorSpinner01 = nullptr;
    SampleShadowManager::DynamicNode* mGeneratorSpinner02 = nullptr;
    stl<SampleShadowManager::DynamicNode*>::vector mFans;
    MinigunObject mMinigun01;
    MinigunObject mMinigun02;
    float mAnimPi = 0.0f;
    float mGammaFactor = 2.2f;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

