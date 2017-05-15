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
        mLightSource.params.block1.w = v.x;
        mLightSource.params.block2.x = v.y;
        mLightSource.params.block2.y = v.z;
        mUpdated = true;
    }
    
    void LightSource::setSpotDirection(const kmVec3 &v)
    {
        mLightSource.params.block4.w = v.x;
        mLightSource.params.block5.x = v.y;
        mLightSource.params.block5.y = v.z;
        mUpdated = true;
    }
    
    void LightSource::setAmbient(const kmVec3 &v)
    {
        mLightSource.params.block2.z = v.x;
        mLightSource.params.block2.w = v.y;
        mLightSource.params.block3.x = v.z;
        mUpdated = true;      
    }
    
    void LightSource::setDiffuse(const kmVec3 &v)
    {
        mLightSource.params.block3.y = v.x;
        mLightSource.params.block3.z = v.y;
        mLightSource.params.block3.w = v.z;
        mUpdated = true;   
    }
    
    void LightSource::setSpecular(const kmVec3 &v)
    {
        mLightSource.params.block4.x = v.x;
        mLightSource.params.block4.y = v.y;
        mLightSource.params.block4.z = v.z;
        mUpdated = true;   
    }
    
    void LightSource::setAttenuation(const kmVec4 &v)
    {
        mLightSource.params.block1.z = v.w;
        mLightSource.params.block6.x = v.x;
        mLightSource.params.block6.y = v.y;
        mLightSource.params.block6.z = v.z;
        mUpdated = true;   
    }
    
    void LightSource::setSpotFactor(const kmVec3 &v)
    {
        mLightSource.params.block5.z = v.x;
        mLightSource.params.block5.w = v.y;
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

    const kmVec3 &LightSource::getPosition() const
    {
        return *(kmVec3 *)&mLightSource.params.block1.w;
    }

    const kmVec3 &LightSource::getSpotDirection() const
    {
        return *(kmVec3 *)&mLightSource.params.block4.w;
    }

    const kmVec3 &LightSource::getAmbient() const
    {
        return *(kmVec3 *)&mLightSource.params.block2.z;
    }

    const kmVec3 &LightSource::getDiffuse() const
    {
        return *(kmVec3 *)&mLightSource.params.block3.y;
    }

    const kmVec3 &LightSource::getSpecular() const
    {
        return *(kmVec3 *)&mLightSource.params.block4.x;
    }

    kmVec4 LightSource::getAttenuation() const
    {
        kmVec4 vec4 = {
            mLightSource.params.block1.z,
            mLightSource.params.block6.x, 
            mLightSource.params.block6.y,
            mLightSource.params.block6.z 
        };
        return vec4;
    }

    kmVec3 LightSource::getSpotFactor() const
    {
        kmVec3 vec3 = {
            mLightSource.params.block5.z,
            mLightSource.params.block5.w,
            0
        };
        return vec3;
    }
}

