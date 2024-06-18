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
#include <ctime>

#include <sample_common/lite3dpp_common.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline.h>
#include <lite3dpp_physics/lite3dpp_physics_scene.h>

namespace lite3dpp {
namespace samples {

static const char *helpString = 
    "Press '+' to increse gamma\n"
    "Press '-' to decrese gamma\n"
    "Press 'l' to enable/disable flashlight\n"
    "Press 'u' to enable/disable SSAO\n"
    "Press 'r' to add light spark\n"
    "Press 'q' to drop light capsule\n"
    "Press 'e' to drop ball\n"
    "Press 'z' to drop heavy cannon ball\n"
    "Press 'g' to disable/enable gravity\n"
    "Press 'space' to jump\n";

class SampleVaultRoom : public Sample
{
public:

    using SampleVaultRoomPipeline = 
        lite3dpp_pipeline::CustomScenePipeline<lite3dpp_pipeline::PipelineDeffered, lite3dpp_phisics::PhysicsScene>;

    SampleVaultRoom() : 
        Sample(helpString)
    {}

    void createScene() override
    {
        auto pipeline = getMain().getResourceManager()->queryResource<SampleVaultRoomPipeline>("Vault_111", 
            "vault_111:pipelines/vault_room.json");
        mVaultScene = &pipeline->getMainScene();
        setMainCamera(&pipeline->getMainCamera());

        mPipeline = pipeline;

        setupPlayer();
        addFlashlight();
    }

    void setupPlayer()
    {
        mPlayer = mVaultScene->addPhysicsObject("Player", "vault_111:objects/Player.json", nullptr, kmVec3 { 0.0, 0.0, 50.0 });
        mVaultScene->attachCamera(&getMainCamera(), mPlayer);
        /* позиция камеры отнгосительно капсулы плеера (приподнимаем камеру)*/
        getMainCamera().setPosition(kmVec3 {0.0, 0.0, 110.0f});
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

    void addSpark()
    {
        auto sparkObject = mVaultScene->addObject("Spark_" + std::to_string(++mObjectCounter), 
            "samples:objects/light_spark.json", nullptr, getMainCamera().getWorldPosition());
        auto node = sparkObject->getLightNode("PointLightSpark.node");
        node->getLight()->setAttenuationConstant(1.0f);
        node->getLight()->setAttenuationLinear(50.0f);
        node->getLight()->setAttenuationQuadratic(130.0f);
        node->getLight()->setRadiance(400000.0f);
        node->getLight()->enabled(true);
    }

    void frameBegin() override
    {
        updateFlashLight();
    }

    void updateCameraVelocity(const kmVec3& velocity, float /*deltaRetard*/) override
    {
        kmVec3 velRelative;
        kmVec3 scale = {15.5f, 15.5f, 15.0f }; 
        auto rotation = getMainCamera().getWorldRotation();
        kmQuaternionMultiplyVec3(&velRelative, &rotation, &velocity);
        kmVec3Mul(&velRelative, &velRelative, &scale);

        /* не модифицируем скорость по z (модет быть в прыжке) */
        velRelative.z = mPlayer->getLinearVelocity().z;
        mPlayer->setLinearVelocity(velRelative);
    }

    void updateFlashLight()
    {
        if (mFlashLight && mFlashLight->getLight()->enabled())
        {
            mFlashLight->setPosition(getMainCamera().getWorldPosition());
            mFlashLight->setRotation(getMainCamera().getWorldRotation());
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
            else if (e->key.keysym.sym == SDLK_KP_PLUS)
            {
                mGamma += 0.02;
                if (mGamma > 3.0)
                    mGamma = 3.0;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.keysym.sym == SDLK_KP_MINUS)
            {
                mGamma -= 0.02;
                if (mGamma < 1.5)
                    mGamma = 1.5;
                mPipeline->setGamma(mGamma);
            }
            else if (e->key.keysym.sym == SDLK_u)
            {
                static bool ssaoEnabled = true;
                ssaoEnabled = !ssaoEnabled;
                mPipeline->enableSSAO(ssaoEnabled);
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                addSpark();
            }
            else if (e->key.keysym.sym == SDLK_q)
            {
                dropObject(mVaultScene->addPhysicsObject("LightCapsule_" + std::to_string(++mObjectCounter), 
                    "vault_111:objects/LightCapsule.json", nullptr,
                    getCameraPositionForObject()), 850.0f);
            }
            else if (e->key.keysym.sym == SDLK_e)
            {
                dropObject(mVaultScene->addPhysicsObject("Ball_" + std::to_string(++mObjectCounter), 
                    "vault_111:objects/Ball.json", nullptr,
                    getCameraPositionForObject()), 850.0f);
            }
            else if (e->key.keysym.sym == SDLK_z)
            {
                dropObject(mVaultScene->addPhysicsObject("CannonBall_" + std::to_string(++mObjectCounter), 
                    "vault_111:objects/CannonBall.json", nullptr,
                    getCameraPositionForObject()), 7000.0f);
            }
            else if (e->key.keysym.sym == SDLK_g)
            {
                mGravityEnabled = !mGravityEnabled;
                if (mGravityEnabled)
                {
                    mVaultScene->setGravity(kmVec3 {0.0, 0.0, -98.0f});
                }
                else
                {
                    mVaultScene->setGravity(kmVec3 {0.0, 0.0, 0.0f});
                }
            }
            else if (e->key.keysym.sym == SDLK_SPACE)
            {
                kmVec3 currVel = mPlayer->getLinearVelocity();
                 /* не модифицируем скорость по x,y (может быть в движении) */
                if (std::abs(currVel.z) < 0.1)
                {
                    currVel.z = 100.0f;
                    mPlayer->setLinearVelocity(currVel);
                }
            }
        }
    }

    kmVec3 getCameraPositionForObject()
    {
        return getMainCamera().transformCoordToWorld(kmVec3 {0.0, 0.0, -50.0});
    }

    void dropObject(lite3dpp_phisics::PhysicsSceneObject *o, float impulse)
    {
        auto impulseRelative = getMainCamera().getWorldDirection();
        kmVec3Scale(&impulseRelative, &impulseRelative, impulse);
        o->applyCentralImpulse(impulseRelative);

        if (mObjects.size() >= 200)
        {
            mVaultScene->removeObject(mObjects.front()->getName());
            mObjects.pop_front();
        }

        mObjects.push_back(o);
    }

private:

    lite3dpp_phisics::PhysicsScene* mVaultScene = nullptr;
    lite3dpp_pipeline::PipelineDeffered* mPipeline = nullptr;
    LightSceneNode* mFlashLight;
    stl<lite3dpp_phisics::PhysicsSceneObject *>::list mObjects;
    lite3dpp_phisics::PhysicsSceneObject *mPlayer = nullptr;
    float mGamma = 2.2;
    int mObjectCounter = 0;
    bool mGravityEnabled = true;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVaultRoom sample;
    return sample.start("vault_111/config/config.json");
}

