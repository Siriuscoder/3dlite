/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2017  Sirius (Korolev Nikita)
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
#include <string.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_light_source.h>

namespace lite3dpp
{
    LightSource::LightSource(const String &name, Main *main) : 
        mName(name),
        mMain(main)
    {
        memset(&mLightSource, 0, sizeof(mLightSource));
        mLightSource.userdata = this;
        /* enabled by default */
        mLightSource.params.block1.y = 1;
    }

    LightSource::LightSource(const lite3d_light_params &ls, Main *main) : 
        mMain(main)
    {
        mLightSource.params = ls;
        mLightSource.userdata = this;
    }

    LightSource::LightSource(const ConfigurationReader &json, Main *main)
    {
        String lightType = json.getString(L"Type", "Point");
        setType(lightType == "Directional" ? LITE3D_LIGHT_DIRECTIONAL : 
            (lightType == "Spot" ? LITE3D_LIGHT_SPOT : LITE3D_LIGHT_POINT));
        
        setPosition(json.getVec3(L"Position"));
        setDirection(json.getVec3(L"SpotDirection"));
        setDiffuse(json.getVec3(L"Diffuse"));
        setLightSize(json.getDouble(L"LightSize"));
        setRadiance(json.getDouble(L"Radiance"));
        
        auto attenuation = json.getObject(L"Attenuation");
        if (!attenuation.isEmpty())
        {
            setAttenuationConstant(attenuation.getDouble(L"Constant"));
            setAttenuationLeaner(attenuation.getDouble(L"Leaner"));
            setAttenuationQuadratic(attenuation.getDouble(L"Quadratic"));
            setInfluenceDistance(attenuation.getDouble(L"InfluenceDistance"));
            setInfluenceMinRadiance(attenuation.getDouble(L"InfluenceMinRadiance"));
        }

        auto spotFactor = json.getObject(L"SpotFactor");
        if (!spotFactor.isEmpty())
        {
            setAngleInnerCone(json.getDouble(L"AngleInnerCone"));
            setAngleOuterCone(json.getDouble(L"AngleOuterCone"));
        }
    }
    
    LightSource::~LightSource()
    {}
    
    void LightSource::setType(uint8_t t)
    {
        mLightSource.params.block1.x = t;
        mUpdated = true;
    }
    
    void LightSource::enabled(bool f)
    {
        mLightSource.params.block1.y = f ? 1 : 0;
        mUpdated = true;  
    }
    
    void LightSource::setPosition(const kmVec3 &v)
    {
        mLightSource.params.block3.x = v.x;
        mLightSource.params.block3.y = v.y;
        mLightSource.params.block3.z = v.z;
        mUpdated = true;
    }
    
    void LightSource::setDirection(const kmVec3 &v)
    {
        mLightSource.params.block4.x = v.x;
        mLightSource.params.block4.y = v.y;
        mLightSource.params.block4.z = v.z;
        mUpdated = true;
    }
    
    void LightSource::setDiffuse(const kmVec3 &v)
    {
        mLightSource.params.block2.x = v.x;
        mLightSource.params.block2.y = v.y;
        mLightSource.params.block2.z = v.z;
        mUpdated = true;   
    }
    
    uint8_t LightSource::getType() const
    {
        return mLightSource.params.block1.x;
    }

    bool LightSource::enabled() const
    {
        return mLightSource.params.block1.y == 1;
    }

    void LightSource::setAttenuationConstant(float value)
    {
        mLightSource.params.block4.w = value;
    }

    void LightSource::setAttenuationLeaner(float value)
    {
        mLightSource.params.block5.x = value;
    }

    void LightSource::setAttenuationQuadratic(float value)
    {
        mLightSource.params.block5.y = value;
    }

    void LightSource::setInfluenceDistance(float value)
    {
        mLightSource.params.block1.z = value;
    }

    void LightSource::setInfluenceMinRadiance(float value)
    {
        mLightSource.params.block1.w = value;
    }

    void LightSource::setRadiance(float value)
    {
        mLightSource.params.block2.w = value;
    }

    void LightSource::setLightSize(float value)
    {
        mLightSource.params.block3.w = value;
    }

    void LightSource::setAngleInnerCone(float value)
    {
        mLightSource.params.block5.z = value;
    }

    void LightSource::setAngleOuterCone(float value)
    {
        mLightSource.params.block5.w = value;
    }

    const kmVec3 &LightSource::getPosition() const
    {
        return *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block3.x);
    }

    const kmVec3 &LightSource::getDirection() const
    {
        return *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block4.x);
    }

    const kmVec3 &LightSource::getDiffuse() const
    {
        return *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block2.x);
    }

    float LightSource::getAttenuationConstant() const
    {
        return mLightSource.params.block4.w;
    }

    float LightSource::getAttenuationLeaner() const
    {
        return mLightSource.params.block5.x;
    }

    float LightSource::getAttenuationQuadratic() const
    {
        return mLightSource.params.block5.y;
    }

    float LightSource::getInfluenceDistance() const
    {
        return mLightSource.params.block1.x;
    }

    float LightSource::getInfluenceMinRadiance() const
    {
        return mLightSource.params.block1.w;
    }

    float LightSource::getRadiance() const
    {
        return mLightSource.params.block2.w;
    }

    float LightSource::getLightSize() const
    {
        return mLightSource.params.block3.w;
    }

    float LightSource::getAngleInnerCone() const
    {
        return mLightSource.params.block5.z;
    }

    float LightSource::getAngleOuterCone() const
    {
        return mLightSource.params.block5.w;
    }
}

