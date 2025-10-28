/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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

#include <tuple>

#include <lite3d/lite3d_shader_params.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_texture.h>
#include <lite3dpp/lite3dpp_texture_buffer.h>
#include <lite3dpp/lite3dpp_ssbo.h>
#include <lite3dpp/lite3dpp_ubo.h>
#include <lite3dpp/lite3dpp_shader_program.h>

namespace lite3dpp
{
#define LITE3D_SHADER_PARAMETER_DECLARE(ptype, intype, outype) \
    void set##ptype##Parameter(const String &name, const intype &value, bool isGlobal = false);\
    outype get##ptype##Parameter(const String &name); \
    static void set##ptype##GlobalParameter(const String &name, const intype &value); \
    static outype get##ptype##GlobalParameter(const String &name);
    
    class LITE3DPP_EXPORT ShaderParameters : public Noncopiable
    {
    public:

        using ParametersStore = stl<String, lite3d_shader_parameter>::unordered_map;
        using UsedGlobalParameters = stl<String>::set;

        ShaderParameters(lite3d_shader_parameters *data);

        bool hasParameter(const String &name, bool isGlobal = false) const;
        lite3d_shader_parameter *getParameter(const String &name, 
            uint8_t type, bool isGlobal, bool createIfNotExist);
        
        static lite3d_shader_parameter *getGlobalParameter(const String &name, 
            uint8_t type, bool createIfNotExist);

        void clear();

        LITE3D_SHADER_PARAMETER_DECLARE(Float, float, float)
        LITE3D_SHADER_PARAMETER_DECLARE(Int, int32_t, int32_t)
        LITE3D_SHADER_PARAMETER_DECLARE(UInt, uint32_t, uint32_t)
        LITE3D_SHADER_PARAMETER_DECLARE(Floatv3, kmVec3, kmVec3)
        LITE3D_SHADER_PARAMETER_DECLARE(Floatv4, kmVec4, kmVec4)
        LITE3D_SHADER_PARAMETER_DECLARE(Floatm3, kmMat3, kmMat3)
        LITE3D_SHADER_PARAMETER_DECLARE(Floatm4, kmMat4, kmMat4)
        LITE3D_SHADER_PARAMETER_DECLARE(Sampler, Texture, Texture *)
        LITE3D_SHADER_PARAMETER_DECLARE(SSBO, SSBO, SSBO *)
        LITE3D_SHADER_PARAMETER_DECLARE(UBO, UBO, UBO *)
        LITE3D_SHADER_PARAMETER_DECLARE(ImageStore, Texture, Texture *)
        
    protected:

        lite3d_shader_parameters *mData = nullptr;
        ParametersStore mParameters;
        UsedGlobalParameters mUsedGlobalParameters;
        static ParametersStore mGlobalParameters;
    };
}