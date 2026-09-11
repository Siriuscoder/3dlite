/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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
        mLightSource.userdata = this;
        mLightSource.params.shadowIndex = -1;
        /* enabled by default */
        enabled(true);
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
        setType(lightType == "Directional" ? LightSourceFlags::TypeDirectional : 
            (lightType == "Spot" ? LightSourceFlags::TypeSpot : 
            (lightType == "Point" ? LightSourceFlags::TypePoint :
            (lightType == "DiskArea" ? LightSourceFlags::TypeDiskArea : 
            (lightType == "RectArea" ? LightSourceFlags::TypeRectArea : LightSourceFlags::TypeUndefined)))));
        
        mName = json.getString(L"Name");
        setPosition(json.getVec3(L"Position"));
        setDirection(json.getVec3(L"Direction", KM_VEC3_NEG_Z));
        setDirectionUP(json.getVec3(L"DirectionUP", KM_VEC3_POS_Y));
        setDiffuse(json.getVec3(L"Diffuse"));
        setRadiance(json.getDouble(L"Radiance"));
        setAreaHeight(json.getDouble(L"AreaHeight"));
        setAreaWidth(json.getDouble(L"AreaWidth"));
        setRadius(json.getDouble(L"Radius"));
        
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

        enabled(true);

        if (json.has(L"ShadowParams"))
        {
            auto shadowParams = json.getObject(L"ShadowParams");
            setFlag(shadowParams.getBool(L"Dynamic", false) ? LightSourceFlags::ShadowDynamic : 
                LightSourceFlags::ShadowStatic);

            auto shadowType = shadowParams.getString(L"Type", "PCF");
            auto shadowTypeFlag = shadowType == "PCF" ? LightSourceFlags::ShadowPcf3x3 : 
                (shadowType == "PCFAdaptive" ? LightSourceFlags::ShadowPcfAdaptive : 
                (shadowType == "PCFPoisson" ? LightSourceFlags::ShadowPoisson :
                (shadowType == "VSM" ? LightSourceFlags::ShadowVSM : LightSourceFlags::TypeUndefined)));
            if (shadowParams.getBool(L"SSS", false))
            {
                shadowTypeFlag = shadowTypeFlag | LightSourceFlags::ShadowSSS;
            }

            setFlag(shadowTypeFlag);
            if (getType() == LightSourceFlags::TypeDirectional)
            {
                mShadowClipParams.leftClipPlane = shadowParams.getDouble(L"LeftClipPlane");
                mShadowClipParams.rightClipPlane = shadowParams.getDouble(L"RightClipPlane");
                mShadowClipParams.bottomClipPlane = shadowParams.getDouble(L"BottomClipPlane");
                mShadowClipParams.topClipPlane = shadowParams.getDouble(L"TopClipPlane");
            }

            mShadowClipParams.nearClipPlane = shadowParams.getDouble(L"NearClipPlane");
            mShadowClipParams.farClipPlane = shadowParams.getDouble(L"FarClipPlane");
        }

        mLightSource.userdata = this;
        mLightSource.params.shadowIndex = -1;
        mLightSourceWorld = mLightSource;
    }
    
    void LightSource::toJson(ConfigurationWriter &writer) const
    {
        if (getType() == LightSourceFlags::TypePoint)
            writer.set(L"Type", "Point");
        else if (getType() == LightSourceFlags::TypeDirectional)
            writer.set(L"Type", "Directional");
        else if (getType() == LightSourceFlags::TypeSpot)
            writer.set(L"Type", "Spot");
        else if (getType() == LightSourceFlags::TypeDiskArea)
            writer.set(L"Type", "DiskArea");
        else if (getType() == LightSourceFlags::TypeRectArea)
            writer.set(L"Type", "RectArea");
        else
            writer.set(L"Type", "Undefined");

        writer.set(L"Name", mName);
        writer.set(L"Diffuse", mLightSource.params.diffuse);
        writer.set(L"Radiance", mLightSource.params.radiance);

        if (getType() == LightSourceFlags::TypePoint || 
            getType() == LightSourceFlags::TypeSpot)
        {
            writer.set(L"Position", mLightSource.params.position);
            writer.set(L"Attenuation", lite3dpp::ConfigurationWriter()
                .set(L"Constant", mLightSource.params.attenuationConstant)
                .set(L"Linear", mLightSource.params.attenuationLinear)
                .set(L"Quadratic", mLightSource.params.attenuationQuadratic)
                .set(L"InfluenceDistance", mLightSource.params.influenceDistance)
                .set(L"InfluenceMinRadiance", mLightSource.params.minRadianceCutOff));

            if (getType() == LightSourceFlags::TypeSpot)
            {
                writer.set(L"SpotFactor", lite3dpp::ConfigurationWriter()
                    .set(L"AngleInnerCone", mLightSource.params.innerCone)
                    .set(L"AngleOuterCone", mLightSource.params.outerCone));
            }

            writer.set(L"Radius", mLightSource.params.radius);
        }

        if (getType() == LightSourceFlags::TypeDirectional || 
            getType() == LightSourceFlags::TypeSpot)
        {
            writer.set(L"Direction", mLightSource.params.direction);
        }

        if (getType() == LightSourceFlags::TypeDiskArea || 
            getType() == LightSourceFlags::TypeRectArea)
        {
            writer.set(L"Position", mLightSource.params.position);
            writer.set(L"Direction", mLightSource.params.direction);
            writer.set(L"DirectionUP", mLightSource.params.directionUP);
            writer.set(L"AreaWidth", mLightSource.params.areaWidth);
            writer.set(L"AreaHeight", mLightSource.params.areaHeight);
        }

        if (mLightSource.params.flags & (LITE3D_LIGHT_SHADOW_DYNAMIC | LITE3D_LIGHT_SHADOW_STATIC))
        {
            lite3dpp::ConfigurationWriter shadowParams;
            shadowParams.set(L"Dynamic", static_cast<bool>(mLightSource.params.flags & LITE3D_LIGHT_SHADOW_DYNAMIC));

            if (mLightSource.params.flags & LITE3D_LIGHT_SHADOW_PCF3x3)
                shadowParams.set(L"Type", "PCF");
            else if (mLightSource.params.flags & LITE3D_LIGHT_SHADOW_PCF_ADAPTIVE)
                shadowParams.set(L"Type", "PCFAdaptive");
            else if (mLightSource.params.flags & LITE3D_LIGHT_SHADOW_POISSON)
                shadowParams.set(L"Type", "PCFPoisson");
            else if (mLightSource.params.flags & LITE3D_LIGHT_SHADOW_VSM)
                shadowParams.set(L"Type", "VSM");
            if (mLightSource.params.flags & LITE3D_LIGHT_SHADOW_SSS)
                shadowParams.set(L"SSS", true);

            if (getType() == LightSourceFlags::TypeDirectional)
            {
                shadowParams.set(L"LeftClipPlane", mShadowClipParams.leftClipPlane);
                shadowParams.set(L"RightClipPlane", mShadowClipParams.rightClipPlane);
                shadowParams.set(L"BottomClipPlane", mShadowClipParams.bottomClipPlane);
                shadowParams.set(L"TopClipPlane", mShadowClipParams.topClipPlane);
            }

            shadowParams.set(L"NearClipPlane", mShadowClipParams.nearClipPlane);
            shadowParams.set(L"FarClipPlane", mShadowClipParams.farClipPlane);
            writer.set(L"ShadowParams", shadowParams);
        }
    }

    void LightSource::setType(LightSourceFlags t)
    {
        mLightSource.params.flags &= ~(LITE3D_LIGHT_POINT | LITE3D_LIGHT_DIRECTIONAL | LITE3D_LIGHT_SPOT | LITE3D_LIGHT_DISK_AREA | LITE3D_LIGHT_RECT_AREA);
        setFlag(t);
    }

    LightSourceFlags LightSource::getFlags() const
    {
        return static_cast<LightSourceFlags>(mLightSource.params.flags);
    }

    void LightSource::setFlag(LightSourceFlags flag)
    {
        mLightSource.params.flags |= static_cast<uint32_t>(flag);
        mUpdated = true;
    }
    
    void LightSource::enabled(bool f)
    {
        if (f)
        {
            mLightSource.params.flags |= LITE3D_LIGHT_ENABLED;
        }
        else
        {
            mLightSource.params.flags &= ~LITE3D_LIGHT_ENABLED;
        }

        mUpdated = true;
    }
    
    void LightSource::setPosition(const kmVec3 &v)
    {
        mLightSource.params.position = v;
        mUpdated = true;
    }
    
    void LightSource::setDirection(const kmVec3 &v)
    {
        mLightSource.params.direction = v;
        mUpdated = true;
    }

    void LightSource::setDirectionUP(const kmVec3 &v)
    {
        mLightSource.params.directionUP = v;
        mUpdated = true;
    }
    
    void LightSource::setDiffuse(const kmVec3 &v)
    {
        mLightSource.params.diffuse = v;
        mUpdated = true;
    }
    
    LightSourceFlags LightSource::getType() const
    {
        return static_cast<LightSourceFlags>(mLightSource.params.flags & 
            (LITE3D_LIGHT_POINT | LITE3D_LIGHT_DIRECTIONAL | LITE3D_LIGHT_SPOT | LITE3D_LIGHT_DISK_AREA | LITE3D_LIGHT_RECT_AREA));
    }

    bool LightSource::enabled() const
    {
        return mLightSource.params.flags & LITE3D_LIGHT_ENABLED;
    }

    void LightSource::setAttenuationConstant(float value)
    {
        mLightSource.params.attenuationConstant = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setAttenuationLinear(float value)
    {
        mLightSource.params.attenuationLinear = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setAttenuationQuadratic(float value)
    {
        mLightSource.params.attenuationQuadratic = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setInfluenceDistance(float value)
    {
        mLightSource.params.influenceDistance = value;
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
        mLightSource.params.minRadianceCutOff = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setRadiance(float value)
    {
        mLightSource.params.radiance = value;
        calcDistanceMinRadiance();
        mUpdated = true;
    }

    void LightSource::setShadowIndex(int32_t value)
    {
        mLightSource.params.shadowIndex = value;
        mUpdated = true;
    }

    void LightSource::setAngleInnerCone(float value)
    {
        mLightSource.params.innerCone = value;
        mUpdated = true;
    }

    void LightSource::setAngleOuterCone(float value)
    {
        mLightSource.params.outerCone = value;
        mUpdated = true;
    }

    void LightSource::setAreaHeight(float value)
    {
        mLightSource.params.areaHeight = value;
        mUpdated = true;
    }

    void LightSource::setAreaWidth(float value)
    {
        mLightSource.params.areaWidth = value;
        mUpdated = true;
    }

    void LightSource::setRadius(float value)
    {
        mLightSource.params.radius = value;
        mUpdated = true;
    }

    const kmVec3 &LightSource::getPosition() const
    {
        return mLightSource.params.position;
    }

    const kmVec3 &LightSource::getDirection() const
    {
        return mLightSource.params.direction;
    }

    const kmVec3 &LightSource::getWorldPosition() const
    {
        return mLightSourceWorld.params.position;
    }

    const kmVec3 &LightSource::getWorldDirection() const
    {
        return mLightSourceWorld.params.direction;
    }

    const kmVec3 &LightSource::getDirectionUP() const
    {
        return mLightSource.params.directionUP;
    }

    const kmVec3 &LightSource::getWorldDirectionUP() const
    {
        return mLightSourceWorld.params.directionUP;
    }

    const kmVec3 &LightSource::getDiffuse() const
    {
        return mLightSource.params.diffuse;
    }

    float LightSource::getAttenuationConstant() const
    {
        return mLightSource.params.attenuationConstant;
    }

    float LightSource::getAttenuationLinear() const
    {
        return mLightSource.params.attenuationLinear;
    }

    float LightSource::getAttenuationQuadratic() const
    {
        return mLightSource.params.attenuationQuadratic;
    }

    float LightSource::getInfluenceDistance() const
    {
        return mLightSource.params.influenceDistance;
    }

    float LightSource::getInfluenceMinRadiance() const
    {
        return mLightSource.params.minRadianceCutOff;
    }

    float LightSource::getRadiance() const
    {
        return mLightSource.params.radiance;
    }

    int32_t LightSource::getShadowIndex() const
    {
        return mLightSource.params.shadowIndex;
    }

    float LightSource::getAngleInnerCone() const
    {
        return mLightSource.params.innerCone;
    }

    float LightSource::getAngleOuterCone() const
    {
        return mLightSource.params.outerCone;
    }

    float LightSource::getAreaHeight() const
    {
        return mLightSource.params.areaHeight;
    }

    float LightSource::getAreaWidth() const
    {
        return mLightSource.params.areaWidth;
    }

    float LightSource::getRadius() const
    {
        return mLightSource.params.radius;
    }

    void LightSource::translateToWorld(const kmMat4 &worldMatrix)
    {
        mLightSourceWorld = mLightSource;

        kmVec3TransformCoord(&mLightSourceWorld.params.position,
            &mLightSourceWorld.params.position, &worldMatrix);

        if (getType() == LightSourceFlags::TypeDirectional || getType() == LightSourceFlags::TypeSpot || 
            getType() == LightSourceFlags::TypeDiskArea || getType() == LightSourceFlags::TypeRectArea)
        {
            kmVec3 direction = KM_VEC3_ZERO;
            kmVec3 directionUP = KM_VEC3_ZERO;
            kmVec3TransformNormal(&direction, &mLightSourceWorld.params.direction, &worldMatrix);
            kmVec3TransformNormal(&directionUP, &mLightSourceWorld.params.directionUP, &worldMatrix);
            kmVec3Normalize(&mLightSourceWorld.params.direction, &direction);
            kmVec3Normalize(&mLightSourceWorld.params.directionUP, &directionUP);
        }

        validate();
    }

    void LightSource::writeToBuffer(BufferBase &buffer)
    {
        buffer.setElement<lite3d_light_params>(index(), &mLightSourceWorld.params);
    }

    lite3d_bounding_vol LightSource::getBoundingVolumeWorld() const
    {
        lite3d_bounding_vol aabb = {};

        if (getType() == LightSourceFlags::TypeSpot)
        {
            if (getAngleOuterCone() < kmDegreesToRadians(100.0f))
            {
                kmVec3 dir;

                float baseRadius = getInfluenceDistance() * tan(getAngleOuterCone() / 2.0);
                kmVec3MulScalar(&dir, &mLightSourceWorld.params.direction, getInfluenceDistance() / 2.0f);
                kmVec3Add(&aabb.sphereCenter, &mLightSourceWorld.params.position, &dir);
                aabb.radius = sqrt(getInfluenceDistance() * getInfluenceDistance() * 0.25f + baseRadius * baseRadius);
                return aabb;
            }
        }

        aabb.radius = getInfluenceDistance();
        aabb.sphereCenter = mLightSourceWorld.params.position;
        return aabb;
    }

    lite3d_bounding_vol LightSource::getBoundingVolume() const
    {
        lite3d_bounding_vol aabb = {};
        aabb.radius = getInfluenceDistance();
        aabb.sphereCenter = mLightSource.params.position;
        return aabb;
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
                mLightSource.params.influenceDistance = std::max(std::max(root1, root2), 0.0f);
            }
        }
    }
}

