/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <algorithm>

#include <lite3dpp_pipeline/lite3dpp_pipeline.h>
#include "lite3dpp_vault_light_anim.h"

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press '+' to increse gamma\n"
    "Press '-' to decrese gamma\n"
    "Press 'o,p' to rotate lamp in gear room\n"
    "            (hold ctrl to reverse)\n"
    "Press 'i' to rotate lamp in reactor room\n"
    "            (hold ctrl to reverse)\n"
    "Press 'l' to enable/disable flashlight\n"
    "Press 'k' to move gear key\n"
    "            (hold ctrl to reverse)\n"
    "Press 'u' to enable/disable SSAO\n";

class SampleVault111 : public Sample
{
public:

    class MinigunObject
    {
    public:

        MinigunObject() = default;

        MinigunObject(Scene* scene, lite3dpp_pipeline::ShadowManager* shadowManager, const String& name) : 
            mMinigunObj(scene->getObject(name))
        {
            mMinigun = mMinigunObj->getNode("Minigun");
            mMinigunBarrel = mMinigunObj->getNode("MinigunBarrel");
            shadowManager->registerHintNodeRecursive(mMinigunObj->getRoot());
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
        SceneNode* mMinigun = nullptr;
        SceneNode* mMinigunBarrel = nullptr;
    };

public:

    SampleVault111() : 
        Sample(helpString)
    {}

    void createScene() override
    {
        mLightAnimEffects = std::make_unique<SampleLightEffectManager>();
        mPipeline = getMain().getResourceManager().queryResource<lite3dpp_pipeline::PipelineDeffered>("Vault_111", 
            "vault_111:pipelines/vault_111.json");
        mVaultScene = &mPipeline->getMainScene();
        setMainCamera(&mPipeline->getMainCamera());

        mShadowManager = mPipeline->getShadowManager();

        setupShadowCasters();
        addFlashlight();
        setupLightAnim();

        mMinigun01 = MinigunObject(mVaultScene, mShadowManager, "MinigunTurret");
        mMinigun02 = MinigunObject(mVaultScene, mShadowManager, "MinigunTurret.001");
        mMinigun02.rotateAngle(KM_VEC3_POS_Z, kmDegreesToRadians(-30.0));

        mGearKey = mVaultScene->getObject("VaultStatic")->getNode("GearKey");
        mShadowManager->registerHintNode(mGearKey);

        mGearKeySpinner = mVaultScene->getObject("VaultStatic")->getNode("GearKeySpinner");
        mShadowManager->registerHintNode(mGearKeySpinner);

        mGeneratorSpinner01 = mVaultScene->getObject("VaultStatic")->getNode("PowerGeneratorSpinner01");
        mShadowManager->registerHintNode(mGeneratorSpinner01);

        mGeneratorSpinner02 = mVaultScene->getObject("VaultStatic")->getNode("PowerGeneratorSpinner02");
        mShadowManager->registerHintNode(mGeneratorSpinner02);

        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.001"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.002"));
        mFans.emplace_back(mVaultScene->getObject("VaultStatic")->getNode("FanRotor.003"));
        
        for (auto fan : mFans)
        {
            mShadowManager->registerHintNode(fan);
        }
    }

    void setupShadowCasters()
    {
        // Установим тень для четырех прожекторов и потом будем их вращать
        // Источники света получаем по ObjectName + NodeName
        mSpot = mVaultScene->getObject("LightSpot")->getNode("LightSpotLamp");
        mSpot01 = mVaultScene->getObject("LightSpot.001")->getNode("LightSpotLamp");
        mSpot02 = mVaultScene->getObject("LightSpot.002")->getNode("LightSpotLamp");
        mSpot03 = mVaultScene->getObject("LightSpot.003")->getNode("LightSpotLamp");

        mShadowManager->registerHintNode(mSpot);
        mShadowManager->registerHintNode(mSpot01);
        mShadowManager->registerHintNode(mSpot02);
        mShadowManager->registerHintNode(mSpot03);

        mShadowManager->newShadowCaster(mVaultScene->getObject("LightSpot")->getLightNode("LightSpotNode"))->getNode()->
            getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);

        mShadowManager->newShadowCaster(mVaultScene->getObject("LightSpot.001")->getLightNode("LightSpotNode"))->getNode()->
            getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);

        mShadowManager->newShadowCaster(mVaultScene->getObject("LightSpot.002")->getLightNode("LightSpotNode"))->getNode()->
            getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);

        mShadowManager->newShadowCaster(mVaultScene->getObject("LightSpot.003")->getLightNode("LightSpotNode"))->getNode()->
            getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);
        
        mShadowManager->newShadowCaster(mVaultScene->getObject("VaultStatic")->getLightNode("RotorSpot"))->getNode()->
            getLight()->setFlag(LightSourceFlags::CastShadowPcfAdaptive);
    }

    void setupLightAnim()
    {
        for (auto &lightNode: mVaultScene->getLights())
        {
            if (lightNode->getName().starts_with("LightSpot") ||
                lightNode->getName().starts_with("HospitalLight") || 
                lightNode->getName() == "LightCageWhiteNode.008" ||
                lightNode->getName() == "LightBoxNode.018" ||
                lightNode->getName() == "LightBoxNode.012" || 
                lightNode->getName() == "LightCageWhiteNode.022" ||
                lightNode->getName() == "LightCeilNode01.002" ||
                lightNode->getName() == "LightCeilNode01.004" ||
                lightNode->getName() == "LightBoxNode.056" ||
                lightNode->getName() == "LightCageWhiteNode.003")
            {
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeTrembling);
            }
            else if (lightNode->getName() == "LightBoxNode.040" ||
                lightNode->getName() == "LightCeilNode01.001" ||
                lightNode->getName() == "LightCageWhiteNode.013" ||
                lightNode->getName() == "LightCageRedNode.001" ||
                lightNode->getName() == "LightCeilNode01.010" ||
                lightNode->getName() == "LightCeilNode01.006")
            {
                // Создаем новый материал для выбранных источников света чтобы сделать эффект мигания каджой отдельной лампочки 
                auto material = getMain().getResourceManager().queryResource<Material>(lightNode->getName() + "_BlinkGlow.material", 
                    "vault_111:materials/VltLightGlow01.json");
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeBlink, material, 1'200'000, 550'000, 80'000);
                // Присваиваем новый натериал к родителю (MeshNode)
                auto parent = lightNode->getParent();
                if (parent)
                {
                    // ВНИМАНИЕ! Для выбранных источников света чанк всегда первый, но для других источников это может быть не так. 
                    MeshSceneNode *meshNode = static_cast<MeshSceneNode*>(parent);
                    meshNode->applyMaterial(1, material);
                }
            }
            else if (lightNode->getName() == "ReactorElectric")
            {
                Material *reactorGlow = getMain().getResourceManager().queryResource<Material>("V111ReactorGlow01.material");
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeTrembling, reactorGlow);
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeBlink, reactorGlow);
            }
            else if (lightNode->getName() == "ReactorElectric.001")
            {
                Material *reactorGlow = getMain().getResourceManager().queryResource<Material>("V111ReactorGlow02.material");
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeTrembling, reactorGlow);
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeBlink, reactorGlow);
            }
            else if (lightNode->getName() == "ReactorElectric.002")
            {
                Material *reactorGlow = getMain().getResourceManager().queryResource<Material>("V111ReactorGlow03.material");
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeTrembling, reactorGlow);
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeBlink, reactorGlow);
            }
            else if (lightNode->getName() == "ReactorElectric.003")
            {
                Material *reactorGlow = getMain().getResourceManager().queryResource<Material>("V111ReactorGlow04.material");
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeTrembling, reactorGlow);
                mLightAnimEffects->registerLight(lightNode, SampleLightEffectManager::EffectTypeBlink, reactorGlow);
            }
        }

        Material *sineWave = getMain().getResourceManager().queryResource<Material>("SineWave01.material");
        mLightAnimEffects->registerLight(nullptr, SampleLightEffectManager::EffectTypeTrembling, sineWave);
    }

    void addFlashlight()
    {
        auto flashLightObject = mVaultScene->addObject("FlashLight", "samples:objects/flashlight.json", nullptr);
        mFlashLight = flashLightObject->getLightNode("FlashLight.node");
        mFlashLight->getLight()->setAttenuationConstant(1.0f);
        mFlashLight->getLight()->setAttenuationLinear(50.0f);
        mFlashLight->getLight()->setAttenuationQuadratic(130.0f);
        mFlashLight->getLight()->setRadiance(800000.0f);
        mFlashLight->getLight()->enabled(false);
    }

    void mainCameraChanged() override
    {
        updateFlashLight();
    }

    void updateFlashLight()
    {
        if (mFlashLight && mFlashLight->getLight()->enabled())
        {
            mFlashLight->setPosition(getMainCamera().getWorldPosition());
            mFlashLight->setRotation(getMainCamera().getWorldRotation());
        }
    }

    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        float animPiNew = mAnimPi + (0.05f * deltaRetard);
        mAnimPi = animPiNew > (2.0 * M_PI) ? animPiNew - (2 * M_PI) : animPiNew;

        float cosA = cos(mAnimPi);
        mMinigun01.animate(cosA * 0.02, 0.13 * deltaRetard);
        mMinigun02.animate(-cosA * 0.02, 0.13 * deltaRetard);
        mSpot03->rotateAngle(KM_VEC3_POS_Z, 0.1 * deltaRetard);

        std::for_each(mFans.begin(), mFans.end(), [deltaRetard](SceneNode* fanRotor)
        {
            fanRotor->rotateAngle(KM_VEC3_POS_Z, 0.07 * deltaRetard);
        });

        mGearKeySpinner->rotateAngle(KM_VEC3_POS_X, 0.1 * deltaRetard);
        mGeneratorSpinner01->rotateAngle(KM_VEC3_POS_Y, 0.025 * deltaRetard);
        mGeneratorSpinner02->rotateAngle(KM_VEC3_POS_Y, -0.025 * deltaRetard);

        mLightAnimEffects->amin(deltaMcs, deltaRetard);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_EVENT_KEY_DOWN)
        {
            if (e->key.key == SDLK_L && mFlashLight)
            {
                static bool flashLightEnabled = false;
                flashLightEnabled = !flashLightEnabled;
                mFlashLight->getLight()->enabled(flashLightEnabled);
                updateFlashLight();
            }
            else if (e->key.key == SDLK_P)
            {
                mSpot->rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.mod & SDL_KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.key == SDLK_I)
            {
                mSpot01->rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.mod & SDL_KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.key == SDLK_O)
            {
                mSpot02->rotateAngle(KM_VEC3_POS_Z, 0.10 * (e->key.mod & SDL_KMOD_LCTRL ? -1.0 : 1.0));
            }
            else if (e->key.key == SDLK_K)
            {
                mGearKey->move(kmVec3{10.0f * (e->key.mod & SDL_KMOD_LCTRL ? -1.0f : 1.0f), 0.0f, 0.0f});
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
            else if (e->key.key == SDLK_KP_PLUS)
            {
                mGamma += 0.02;
                if (mGamma > 3.0)
                    mGamma = 3.0;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.key == SDLK_KP_MINUS)
            {
                mGamma -= 0.02;
                if (mGamma < 1.5)
                    mGamma = 1.5;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.key == SDLK_U)
            {
                static bool ssaoEnabled = true;
                ssaoEnabled = !ssaoEnabled;
                mPipeline->enableSSAO(ssaoEnabled);
            }
        }
    }

private:

    Scene* mVaultScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    lite3dpp_pipeline::ShadowManager* mShadowManager;
    std::unique_ptr<SampleLightEffectManager> mLightAnimEffects;
    LightSceneNode* mFlashLight = nullptr;
    SceneNode* mSpot;
    SceneNode* mSpot01;
    SceneNode* mSpot02;
    SceneNode* mSpot03;
    SceneNode* mGearKey = nullptr;
    SceneNode* mGearKeySpinner = nullptr;
    SceneNode* mGeneratorSpinner01 = nullptr;
    SceneNode* mGeneratorSpinner02 = nullptr;
    stl<SceneNode*>::vector mFans;
    MinigunObject mMinigun01;
    MinigunObject mMinigun02;
    float mAnimPi = 0.0f;
    float mGamma = 2.2f;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

