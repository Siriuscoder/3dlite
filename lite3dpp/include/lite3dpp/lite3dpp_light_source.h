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

#include <optional>

#include <lite3d/lite3d_lighting.h>
#include <lite3d/lite3d_frustum.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_config_writer.h>
#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_buffer_base.h>

namespace lite3dpp
{
    enum class LightSourceFlags : uint32_t
    {
        TypePoint = LITE3D_LIGHT_POINT,
        TypeDirectional = LITE3D_LIGHT_DIRECTIONAL,
        TypeSpot = LITE3D_LIGHT_SPOT,
        Enabled = LITE3D_LIGHT_ENABLED,
        CastShadow = LITE3D_LIGHT_CASTSHADOW,
        CastShadowPcf3x3 = LITE3D_LIGHT_CASTSHADOW_PCF3x3,
        CastShadowPcfAdaptive = LITE3D_LIGHT_CASTSHADOW_PCF_ADAPTIVE,
        CastShadowPoisson = LITE3D_LIGHT_CASTSHADOW_POISSON,
        CastShadowSSS = LITE3D_LIGHT_CASTSHADOW_SSS
    };

    LITE3D_DECLARE_ENUM_OPERATORS(LightSourceFlags);

    class LITE3DPP_EXPORT LightSource : public Noncopiable, public Manageable
    {
    public:

        LightSource(const String &name);
        LightSource(const String &name, const lite3d_light_params &ls);
        LightSource(const ConfigurationReader &json);
        ~LightSource() = default;

        inline lite3d_light_source *getPtr()
        { return &mLightSource; }
        inline const lite3d_light_source *getPtr() const
        { return &mLightSource; }

        inline const String &getName() const
        { return mName; }
        
        inline void setBufferIndex(uint32_t i)
        { mBufferIndex = i; }     
        inline int32_t index() const
        { return mBufferIndex; }
        inline void index(uint32_t i)
        { mBufferIndex = i; }
        inline bool isUpdated() const 
        { return mUpdated; }
        inline void validate()
        { mUpdated = false; }

        void toJson(ConfigurationWriter &writer) const;
        
        void setType(LightSourceFlags t);
        void setFlag(LightSourceFlags flag);
        void enabled(bool f);
        void setPosition(const kmVec3 &v);
        void setDirection(const kmVec3 &v);
        void setDiffuse(const kmVec3 &v);
        void setAttenuationConstant(float value);
        void setAttenuationLinear(float value);
        void setAttenuationQuadratic(float value);
        void setInfluenceDistance(float value);
        void setInfluenceMinRadiance(float value);
        void setRadiance(float value);
        void setShadowIndex(uint32_t value);
        void setAngleInnerCone(float value);
        void setAngleOuterCone(float value);

        LightSourceFlags getType() const;
        LightSourceFlags getFlags() const;
        bool enabled() const;
        const kmVec3 &getPosition() const;
        const kmVec3 &getWorldPosition() const;
        const kmVec3 &getDirection() const;
        const kmVec3 &getWorldDirection() const;
        const kmVec3 &getDiffuse() const;
        float getAttenuationConstant() const;
        float getAttenuationLinear() const;
        float getAttenuationQuadratic() const;
        float getInfluenceDistance() const;
        float getInfluenceMinRadiance() const;
        float getRadiance() const;
        uint32_t getShadowIndex() const;
        float getAngleInnerCone() const;
        float getAngleOuterCone() const;

        void translateToWorld(const kmMat4 &worldView);
        void writeToBuffer(BufferBase &buffer);
        lite3d_bounding_vol getBoundingVolumeWorld() const;
        lite3d_bounding_vol getBoundingVolume() const;

    private:

        void calcDistanceMinRadiance();

        String mName;
        lite3d_light_source mLightSource = {};
        lite3d_light_source mLightSourceWorld = {};
        uint32_t mBufferIndex = 0;
        bool mUpdated = false;
        std::optional<float> mInfluenceDistance;
    };
}

