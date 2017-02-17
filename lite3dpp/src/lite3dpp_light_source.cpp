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
        mLightSource.params.flags.y = 1;
    }
    
    LightSource::~LightSource()
    {}
    
    void LightSource::setType(uint8_t t)
    {
        mLightSource.params.flags.x = t;
        mUpdated = true;
    }
    
    void LightSource::enabled(bool f)
    {
        mLightSource.params.flags.y = f ? 1 : 0;
        mUpdated = true;  
    }
    
    void LightSource::setPosition(const kmVec4 &v)
    {
        mLightSource.params.position = v;
        mUpdated = true;    
    }
    
    void LightSource::setSpotDirection(const kmVec4 &v)
    {
        mLightSource.params.spotDirection = v;
        mUpdated = true;
    }
    
    void LightSource::setAmbient(const kmVec4 &v)
    {
        mLightSource.params.ambient = v;
        mUpdated = true;      
    }
    
    void LightSource::setDiffuse(const kmVec4 &v)
    {
        mLightSource.params.diffuse = v;
        mUpdated = true;   
    }
    
    void LightSource::setSpecular(const kmVec4 &v)
    {
        mLightSource.params.specular = v;
        mUpdated = true;   
    }
    
    void LightSource::setAttenuation(const kmVec4 &v)
    {
        mLightSource.params.attenuation = v;
        mUpdated = true;   
    }
    
    void LightSource::setSpotFactor(const kmVec4 &v)
    {
        mLightSource.params.spotFactor = v;
        mUpdated = true;   
    }
}

