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
#pragma once

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class LightAminPreset
{
public:

    virtual ~LightAminPreset() = default;

    LightAminPreset(LightSceneNode *node, Material *material) : 
        mNode(node),
        mMaterial(material)
    {
        if (mMaterial && mMaterial->hasParameter("EmissionStrength"))
        {
            mEmissionStrength = mMaterial->getFloatParameter("EmissionStrength");
        }
    }

    virtual void anim(uint64_t deltaMcs, float deltaRetard) = 0;

protected:
    LightSceneNode *mNode = nullptr;
    Material *mMaterial = nullptr;
    float mEmissionStrength = 0.0f;
};

class LightAminTremblingPreset : public LightAminPreset
{
public:
    LightAminTremblingPreset(LightSceneNode *node, Material *material) : 
        LightAminPreset(node, material)
    {
        float rNum = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        mAnimPi = rNum * 2.0 * M_PI;
        mMaxRadiance = node->getLight()->getRadiance();
    }

    void anim(uint64_t deltaMcs, float deltaRetard) override
    {
        SDL_assert(mNode);
        float animPiNew = mAnimPi + (0.3f * deltaRetard);
        mAnimPi = animPiNew > (2.0 * M_PI) ? animPiNew - (2 * M_PI) : animPiNew;
        float k = 1.0 - cos(mAnimPi) * 0.15;
        mNode->getLight()->setRadiance(mMaxRadiance * k);

        if (mMaterial && mNode->getLight()->enabled())
        {
            if (mMaterial->hasParameter("Alpha"))
            {
                mMaterial->setFloatParameter(2, "Alpha", k);
            }

            if (mMaterial->hasParameter("EmissionStrength"))
            {
                float emission = mEmissionStrength * k;
                mMaterial->setFloatParameter(2, "EmissionStrength", emission);
            }
        }
    }

private:

    float mAnimPi;
    float mMaxRadiance;
};

class LightAminBlinkPreset : public LightAminPreset
{
public:
    LightAminBlinkPreset(LightSceneNode *node, Material *material) : 
        LightAminPreset(node, material)
    {}

    void anim(uint64_t deltaMcs, float deltaRetard) override
    {
        SDL_assert(mNode);
        counterMcs += deltaMcs;
        if (counterMcs > thresholdMcs)
        {
            counterMcs = 0;

            float frand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            thresholdMcs = frand * (mNode->getLight()->enabled() ? offMaxThresholdMcs : onMaxThresholdMcs) + minThresholdMcs;
            mNode->getLight()->enabled(!mNode->getLight()->enabled());

            if (mMaterial)
            {
                if (mMaterial->hasParameter("Alpha"))
                {
                    mMaterial->setFloatParameter(2, "Alpha", mNode->getLight()->enabled() ? 1.0 : 0.0);
                }

                if (mMaterial->hasParameter("EmissionStrength"))
                {
                    float emission = mEmissionStrength * (mNode->getLight()->enabled() ? 1.0 : 0.0);
                    mMaterial->setFloatParameter(2, "EmissionStrength", emission);
                }
            }
        }
    }

private:

    uint64_t counterMcs = 0;
    uint64_t thresholdMcs = 0;

    uint64_t onMaxThresholdMcs = 4'200'000;
    uint64_t offMaxThresholdMcs = 850'000;
    uint64_t minThresholdMcs = 100'000;
};

class SampleLightEffectManager
{
public:

    enum EffectType
    {
        Trembling = 1,
        Blink = 2
    };

    void registerLight(LightSceneNode *node, EffectType type, Material *material = nullptr)
    {
        if (type == Trembling)
            mLights.emplace_back(std::make_unique<LightAminTremblingPreset>(node, material));
        else if (type == Blink)
            mLights.emplace_back(std::make_unique<LightAminBlinkPreset>(node, material));
    }

    void amin(uint64_t deltaMcs, float deltaRetard)
    {
        for (auto &l: mLights)
        {
            l->anim(deltaMcs, deltaRetard);
        }
    }

private:

    std::vector<std::unique_ptr<LightAminPreset>> mLights;
};

}}
