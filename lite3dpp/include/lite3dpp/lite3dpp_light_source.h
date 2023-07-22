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
#pragma once

#include <lite3d/lite3d_lighting.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT LightSource : public Noncopiable, public Manageable
    {
    public:

        LightSource(const String &name, Main *main);
        LightSource(const lite3d_light_params &ls, Main *main);
        LightSource(const ConfigurationReader &json, Main *main);
        ~LightSource();

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
        
        void setType(uint8_t t);
        void enabled(bool f);
        void setPosition(const kmVec3 &v);
        void setDirection(const kmVec3 &v);
        void setDiffuse(const kmVec3 &v);
        void setAttenuationConstant(float value);
        void setAttenuationLeaner(float value);
        void setAttenuationQuadratic(float value);
        void setInfluenceDistance(float value);
        void setInfluenceMinRadiance(float value);
        void setRadiance(float value);
        void setLightSize(float value);
        void setAngleInnerCone(float value);
        void setAngleOuterCone(float value);

        uint8_t getType() const;
        bool enabled() const;
        const kmVec3 &getPosition() const;
        const kmVec3 &getDirection() const;
        const kmVec3 &getDiffuse() const;
        float getAttenuationConstant() const;
        float getAttenuationLeaner() const;
        float getAttenuationQuadratic() const;
        float getInfluenceDistance() const;
        float getInfluenceMinRadiance() const;
        float getRadiance() const;
        float getLightSize() const;
        float getAngleInnerCone() const;
        float getAngleOuterCone() const;

    private:

        String mName;
        Main *mMain;
        lite3d_light_source mLightSource;
        uint32_t mBufferIndex;
        bool mUpdated;
    };
}

