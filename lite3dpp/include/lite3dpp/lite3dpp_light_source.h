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
#include <lite3dpp/lite3dpp_scene_object.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT LightSource : public Noncopiable, public Manageable
    {
    public:

        LightSource(const String &name, Main *main);
        ~LightSource();

        inline lite3d_light_source *getPtr()
        { return &mLightSource; }
        inline const lite3d_light_source *getPtr() const
        { return &mLightSource; }
        
        inline void setBufferIndex(int32_t i)
        { mBufferIndex = i; }     
        inline int32_t getBufferIndex() const
        { return mBufferIndex; }

    private:

        String mName;
        Main *mMain;
        lite3d_light_source mLightSource;
        int32_t mBufferIndex;
    };
}

