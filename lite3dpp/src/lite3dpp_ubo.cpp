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
#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_ubo.h>

namespace lite3dpp
{
    UBO::UBO(const String &name, const String &path, Main *main) : 
        VBOResource(name, path, main, AbstractResource::UNIFORM_BUFFER)
    {}
    
    void UBO::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (!lite3d_ubo_init(getPtr()))
            LITE3D_THROW(getName() << ": failed to create SSBO");
        
        VBOResource::loadFromConfigImpl(helper);
    }
}

