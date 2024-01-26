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
#include <string.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_light_source.h>

namespace lite3dpp
{
    LightSource::LightSource(const String &name) : 
        mName(name)
    {
        memset(&mLightSource, 0, sizeof(mLightSource));
        mLightSource.userdata = this;
        /* enabled by default */
        mLightSource.params.block1.y = 1.0f;
        mLightSource.params.block3.w = -1.0f;
        mLightSourceWorld = mLightSource;
    }

    LightSource::LightSource(const String &name, const lite3d_light_params &ls) : 
        mName(name)
    {
        mLightSource.params = ls;
        mLightSource.userdata = this;
        mLightSourceWorld = mLightSource;
    }

    LightSource::LightSource(const ConfigurationReader &json)
    {
        String lightType = json.getString(L"Type", "Point");
        setType(lightType == "Directional" ? LITE3D_LIGHT_DIRECTIONAL : 
            (lightType == "Spot" ? LITE3D_LIGHT_SPOT : LITE3D_LIGHT_POINT));
        
        mName = json.getString(L"Name");
        setPosition(json.getVec3(L"Position"));
        setDirection(json.getVec3(L"Direction"));
        setDiffuse(json.getVec3(L"Diffuse"));
        setRadiance(json.getDouble(L"Radiance"));
        
        auto attenuation = json.getObject(L"Attenuation");
        if (!attenuation.isEmpty())
        {
            setAttenuationConstant(attenuation.getDouble(L"Constant"));
            setAttenuationLinear(attenuation.getDouble(L"Linear"));
            setAttenuationQuadratic(attenuation.getDouble(L"Quadratic"));
            setInfluenceDistance(attenuation.getDouble(L"InfluenceDistance"));
            setInfluenceMinRadiance(attenuation.getDouble(L"InfluenceMinRadiance"));
        }

        auto spotFactor = json.getObject(L"SpotFactor");
        if (!spotFactor.isEmpty())
        {
            setAngleInnerCone(spotFactor.getDouble(L"AngleInnerCone"));
            setAngleOuterCone(spotFactor.getDouble(L"AngleOuterCone"));
        }

        mLightSource.params.block1.y = 1.0f;
        mLightSource.params.block3.w = -1.0f;
        mLightSource.userdata = this;
        mLightSourceWorld = mLightSource;
    }
    
    void LightSource::toJson(ConfigurationWriter &writer) const
    {
        if (getType() == LITE3D_LIGHT_POINT)
            writer.set(L"Type", "Point");
        else if (getType() == LITE3D_LIGHT_DIRECTIONAL)
            writer.set(L"Type", "Directional");
        else if (getType() == LITE3D_LIGHT_SPOT)
            writer.set(L"Type", "Spot");
        else
            writer.set(L"Type", "Undefined");

        writer.set(L"Name", mName);
        writer.set(L"Diffuse", *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block2.x));
        writer.set(L"Radiance", mLightSource.params.block2.w);

        if (getType() == LITE3D_LIGHT_POINT || 
            getType() == LITE3D_LIGHT_SPOT)
        {
            writer.set(L"Position", *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block3.x));
            writer.set(L"LightSize", mLightSource.params.block3.w);
            writer.set(L"Attenuation", lite3dpp::ConfigurationWriter()
                .set(L"Constant", mLightSource.params.block4.w)
                .set(L"Linear", mLightSource.params.block5.x)
                .set(L"Quadratic", mLightSource.params.block5.y)
                .set(L"InfluenceDistance", mLightSource.params.block1.z)
                .set(L"InfluenceMinRadiance", mLightSource.params.block1.w));

            if (getType() == LITE3D_LIGHT_SPOT)
            {
                writer.set(L"SpotFactor", lite3dpp::ConfigurationWriter()
                    .set(L"AngleInnerCone", mLightSource.params.block5.z)
                    .set(L"AngleOuterCone", mLightSource.params.block5.w));
            }
        }

        if (getType() == LITE3D_LIGHT_DIRECTIONAL || 
            getType() == LITE3D_LIGHT_SPOT)
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
        mLightSource.params.block1.y = f ? 1.0f : 0.0f;
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
        if (near(mLightSource.params.block1.x, 1.0f))
            return LITE3D_LIGHT_POINT;
        else if (near(mLightSource.params.block1.x, 2.0f))
            return LITE3D_LIGHT_DIRECTIONAL;
        else if (near(mLightSource.params.block1.x, 3.0f))
            return LITE3D_LIGHT_SPOT;

        return LITE3D_LIGHT_UNDEFINED;
    }

    bool LightSource::enabled() const
    {
        return near(mLightSource.params.block1.y, 1.0f);
    }

    void LightSource::setAttenuationConstant(float value)
    {
        mLightSource.params.block4.w = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setAttenuationLinear(float value)
    {
        mLightSource.params.block5.x = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setAttenuationQuadratic(float value)
    {
        mLightSource.params.block5.y = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setInfluenceDistance(float value)
    {
        mLightSource.params.block1.z = value;
        if (nonzero(value))
        {
            mInfluenceDistance = value;
        }
        else
        {
            mInfluenceDistance = std::nullopt;
            calcDistanceMinRadiance();
        }

        mUpdated = true;
    }

    void LightSource::setInfluenceMinRadiance(float value)
    {
        mLightSource.params.block1.w = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setRadiance(float value)
    {
        mLightSource.params.block2.w = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setUserIndex(int32_t value)
    {
        mLightSource.params.block3.w = static_cast<float>(value);
        mUpdated = true;
    }

    void LightSource::setAngleInnerCone(float value)
    {
        mLightSource.params.block5.z = value;
        mUpdated = true;
    }

    void LightSource::setAngleOuterCone(float value)
    {
        mLightSource.params.block5.w = value;
        mUpdated = true;
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

    float LightSource::getAttenuationLinear() const
    {
        return mLightSource.params.block5.x;
    }

    float LightSource::getAttenuationQuadratic() const
    {
        return mLightSource.params.block5.y;
    }

    float LightSource::getInfluenceDistance() const
    {
        return mLightSource.params.block1.z;
    }

    float LightSource::getInfluenceMinRadiance() const
    {
        return mLightSource.params.block1.w;
    }

    float LightSource::getRadiance() const
    {
        return mLightSource.params.block2.w;
    }

    int32_t LightSource::getUserIndex() const
    {
        return static_cast<int32_t>(round(mLightSource.params.block3.w));
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

    lite3d_bounding_vol LightSource::getBoundingVolumeWorld() const
    {
        lite3d_bounding_vol volume = {};
        volume.radius = getInfluenceDistance();
        volume.sphereCenter = *reinterpret_cast<const kmVec3 *>(&mLightSourceWorld.params.block3.x);
        return volume;
    }

    lite3d_bounding_vol LightSource::getBoundingVolume() const
    {
        lite3d_bounding_vol volume = {};
        volume.radius = getInfluenceDistance();
        volume.sphereCenter = *reinterpret_cast<const kmVec3 *>(&mLightSource.params.block3.x);
        return volume;
    }

    void LightSource::calcDistanceMinRadiance()
    {
        if (!mInfluenceDistance && nonzero(getInfluenceMinRadiance()))
        {
            const auto a = getAttenuationQuadratic();
            const auto b = getAttenuationLinear();
            const auto c = getAttenuationConstant() - (getRadiance() / getInfluenceMinRadiance());

            // R / (a * d * d + b * d + c) = Rmin
            // a * d * d + b * d + (c - R / Rmin) = 0

            const auto D = (b * b) - (4 * a * c);
            if (iszero(D) || D > 0.0f)
            {
                const auto nomRoot1 = -b + std::sqrt(D);
                const auto nomRoot2 = -b - std::sqrt(D);
                const auto root1 = nonzero(nomRoot1) ? (2.0f * c) / nomRoot1 : -std::numeric_limits<float>::max();
                const auto root2 = nonzero(nomRoot2) ? (2.0f * c) / nomRoot2 : -std::numeric_limits<float>::max();
                mLightSource.params.block1.z = std::max(std::max(root1, root2), 0.0f);
            }
        }
    }
}

