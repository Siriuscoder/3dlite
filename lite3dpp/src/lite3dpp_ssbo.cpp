/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <lite3dpp/lite3dpp_ssbo.h>

namespace lite3dpp
{
    SSBO::SSBO(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::SHADER_STORAGE),
        VBO(mSSBO)
    {
        mSSBO.userdata = this;
    }
    
    SSBO::~SSBO()
    {
        lite3d_vbo_purge(&mSSBO);
    }
    
    size_t SSBO::usedVideoMemBytes() const
    {
        return mSSBO.size;
    }
    
    void SSBO::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (!lite3d_ssbo_init(&mSSBO))
            LITE3D_THROW(getName() << ": failed to create SSBO");
        
        size_t size;
        if ((size = helper.getInt(L"Size", 0)) > 0)
        {
            if (!lite3d_vbo_buffer(&mSSBO, NULL, size, helper.
                getBool(L"Dynamic", true) ? LITE3D_VBO_DYNAMIC_DRAW : LITE3D_VBO_STATIC_DRAW))
                LITE3D_THROW(getName() << ": failed to allocate SSBO to " << size << " bytes");
        }
    }
    
    void SSBO::unloadImpl()
    {
        if (bufferSizeBytes() > 0)
        {
            /* load data into host memory */
            getData(mSSBOData, 0, bufferSizeBytes());
            lite3d_vbo_buffer(&mSSBO, NULL, 0, mSSBO.access);
        }
    }
    
    void SSBO::reloadFromConfigImpl(const ConfigurationReader &helper)
    {
        if (mSSBOData.size() > 0)
        {
            if (!lite3d_vbo_buffer(&mSSBO, &mSSBOData[0], mSSBOData.size(), mSSBO.access))
                LITE3D_THROW(getName() << ": failed to reload vertex buffer");
        }
    }
}

