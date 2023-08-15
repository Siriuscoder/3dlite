/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <iostream>
#include <initializer_list>
#include <gtest/gtest.h>

#include <lite3dpp/lite3dpp_main.h>

class LightSourceParams
{
public:

    float attenuationConstant = 0.0f; 
    float attenuationLinear = 0.0f;
    float attenuationQuadratic = 0.0f;
    float radiance = 1.0f;
    float influenceMinRadiance = 0.001f;
};

class LightSourceAttenuation_Test : public ::testing::TestWithParam<LightSourceParams>
{
public:

    LightSourceAttenuation_Test() : 
        mLight("Test1", nullptr)
    {}

protected:

    float calcRadiance() const
    {
        const auto distance = mLight.getInfluenceDistance();
        const auto attenuation = mLight.getAttenuationQuadratic() * distance * distance + 
            mLight.getAttenuationLinear() * distance + 
            mLight.getAttenuationConstant();

        return mLight.getRadiance() / attenuation;
    }

    lite3dpp::LightSource mLight;
};

TEST_P(LightSourceAttenuation_Test, CalcInfluenceByAttenuation01)
{
    const auto &params = GetParam();
    mLight.setAttenuationConstant(params.attenuationConstant);
    mLight.setAttenuationLinear(params.attenuationLinear);
    mLight.setAttenuationQuadratic(params.attenuationQuadratic);
    mLight.setRadiance(params.radiance);
    mLight.setInfluenceMinRadiance(params.influenceMinRadiance);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));
}

TEST_P(LightSourceAttenuation_Test, CalcInfluenceByAttenuation02)
{
    const auto &params = GetParam();
    mLight.setRadiance(params.radiance);
    mLight.setInfluenceMinRadiance(params.influenceMinRadiance);
    mLight.setAttenuationConstant(params.attenuationConstant);
    mLight.setAttenuationLinear(params.attenuationLinear);
    mLight.setAttenuationQuadratic(params.attenuationQuadratic);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));
}

TEST_P(LightSourceAttenuation_Test, CalcInfluenceByAttenuation03)
{
    const auto &params = GetParam();
    mLight.setAttenuationConstant(params.attenuationConstant);
    mLight.setAttenuationLinear(params.attenuationLinear);
    mLight.setAttenuationQuadratic(params.attenuationQuadratic);
    mLight.setInfluenceMinRadiance(params.influenceMinRadiance);
    mLight.setRadiance(params.radiance);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));
}

TEST_P(LightSourceAttenuation_Test, CalcInfluenceByAttenuation04)
{
    const auto &params = GetParam();
    mLight.setInfluenceMinRadiance(params.influenceMinRadiance);
    mLight.setRadiance(params.radiance);

    mLight.setAttenuationLinear(6.5);
    mLight.setAttenuationQuadratic(0.70);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));

    mLight.setAttenuationLinear(8.5);
    mLight.setAttenuationQuadratic(0.002);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));

    mLight.setAttenuationLinear(0.0);
    mLight.setAttenuationQuadratic(0.05);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));

    mLight.setAttenuationLinear(0.1);
    mLight.setAttenuationQuadratic(6.10);
    EXPECT_TRUE(lite3dpp::near(mLight.getInfluenceMinRadiance(), calcRadiance()));
}

INSTANTIATE_TEST_SUITE_P(
    LightSourceAttenuation,
    LightSourceAttenuation_Test,
        ::testing::Values(
            LightSourceParams { 0.0f, 10.5f, 0.0f, 50.0f, 0.001f },
            LightSourceParams { 0.0f, 40.5f, 0.0f, 50.0f, 0.001f },
            LightSourceParams { 0.0f, 10.5f, 0.05f, 50.0f, 0.001f },
            LightSourceParams { 0.0f, 40.5f, 0.70f, 50.0f, 0.001f },
            LightSourceParams { 0.0f,  0.0f, 0.05f, 50.0f, 0.001f },
            LightSourceParams { 0.0f,  0.0f, 0.70f, 50.0f, 0.001f },
            LightSourceParams { 10.0f, 10.5f, 0.0f, 50.0f, 0.001f },
            LightSourceParams { 20.0f, 40.5f, 0.0f, 50.0f, 0.001f },
            LightSourceParams { 5.0f, 10.5f, 0.05f, 100.0f, 0.001f },
            LightSourceParams { 7.0f, 40.5f, 0.70f, 500.0f, 0.001f },
            LightSourceParams { 10.0f,  0.0f, 0.05f, 1.0f, 0.001f },
            LightSourceParams { 1.0f,  0.0f, 0.70f, 0.5f, 0.001f },
            LightSourceParams { 1.0f,  10.0f, 0.70f, 0.5f, 0.1f }
        )
    );