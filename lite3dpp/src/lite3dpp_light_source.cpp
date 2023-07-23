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
        mLightSourceWorld = mLightSource;
    }

    LightSource::LightSource(const lite3d_light_params &ls, Main *main) : 
        mMain(main)
    {
        mLightSource.params = ls;
        mLightSource.userdata = this;
        mLightSourceWorld = mLightSource;
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

        mLightSource.userdata = this;
        mLightSourceWorld = mLightSource;
    }
    
    void LightSource::toJson(ConfigurationWriter &writer) const
    {
        if (mLightSource.params.block1.x == LITE3D_LIGHT_POINT)
            writer.set(L"Type", "Point");
        else if (mLightSource.params.block1.x == LITE3D_LIGHT_DIRECTIONAL)
            writer.set(L"Type", "Directional");
        else if (mLightSource.params.block1.x == LITE3D_LIGHT_SPOT)
            writer.set(L"Type", "Spot");
        else
            writer.set(L"Type", "Undefined");

        writer.set(L"Name", mName);
        writer.set(L"Diffuse", *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block2.x));
        writer.set(L"Radiance", mLightSource.params.block2.w);

        if (mLightSource.params.block1.x == LITE3D_LIGHT_POINT || 
            mLightSource.params.block1.x == LITE3D_LIGHT_SPOT)
        {
            writer.set(L"Position", *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block3.x));
            writer.set(L"LightSize", mLightSource.params.block3.w);
            writer.set(L"Attenuation", lite3dpp::ConfigurationWriter()
                .set(L"Constant", mLightSource.params.block4.w)
                .set(L"Linear", mLightSource.params.block5.x)
                .set(L"Quadratic", mLightSource.params.block5.y)
                .set(L"InfluenceDistance", mLightSource.params.block1.z)
                .set(L"InfluenceMinRadiance", mLightSource.params.block1.w));

            if (mLightSource.params.block1.x == LITE3D_LIGHT_SPOT)
            {
                writer.set(L"SpotFactor", lite3dpp::ConfigurationWriter()
                    .set(L"AngleInnerCone", mLightSource.params.block5.z)
                    .set(L"AngleOuterCone", mLightSource.params.block5.w));
            }
        }

        if (mLightSource.params.block1.x == LITE3D_LIGHT_DIRECTIONAL || 
            mLightSource.params.block1.x == LITE3D_LIGHT_SPOT)
        {
            writer.set(L"Direction", *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block4.x));
        }      
    }

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

    const kmVec3 &LightSource::getPositionWorld() const
    {
        return *reinterpret_cast<const kmVec3 *>(&mLightSourceWorld.params.block3.x);
    }

    const kmVec3 &LightSource::getDirectionWorld() const
    {
        return *reinterpret_cast<const kmVec3 *>(&mLightSourceWorld.params.block4.x);
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

    void LightSource::translateToWorld(const kmMat4 &worldView)
    {
        mLightSourceWorld = mLightSource;
        kmVec3TransformCoord(reinterpret_cast<kmVec3 *>(&mLightSourceWorld.params.block3.x), 
            reinterpret_cast<kmVec3 *>(&mLightSourceWorld.params.block3.x), &worldView);

        if (getType() == LITE3D_LIGHT_DIRECTIONAL || getType() == LITE3D_LIGHT_SPOT)
        {
            kmVec3 direction = KM_VEC3_ZERO;
            kmVec3TransformNormal(&direction, 
                reinterpret_cast<kmVec3 *>(&mLightSourceWorld.params.block4.x), &worldView);
            kmVec3Normalize(reinterpret_cast<kmVec3 *>(&mLightSourceWorld.params.block4.x), &direction);
        }

        validate();
    }

    void LightSource::writeToBuffer(BufferBase &buffer)
    {
        buffer.setElement<lite3d_light_params>(index(), &mLightSourceWorld.params);
    }

    lite3d_bounding_vol LightSource::getBoundingVolume() const
    {
        lite3d_bounding_vol volume = {};
        volume.radius = getInfluenceDistance();
        volume.sphereCenter = *reinterpret_cast<const kmVec3 *>(&mLightSourceWorld.params.block3.x);
        return volume;
    }
}

