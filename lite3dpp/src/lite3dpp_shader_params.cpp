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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_shader_params.h>

lite3dpp::ShaderParameters::ParametersStore lite3dpp::ShaderParameters::mGlobalParameters;

#define LITE3D_IMPLEMENT_SHADER_PARAMETER(ptype, intype, outtype, enums, valto, valfrom) \
    void ShaderParameters::set##ptype##Parameter(const String &name, const intype &value, bool isGlobal) \
    { \
        lite3d_shader_parameter *parameterPtr = getParameter(name, enums, isGlobal, true); \
        parameterPtr->parameter.valto = value; \
    } \
    outtype ShaderParameters::get##ptype##Parameter(const String &name) \
    { \
        lite3d_shader_parameter *parameterPtr = getParameter(name, enums, false, false); \
        if (parameterPtr->type != enums) \
            LITE3D_THROW("Parameter " << name << " type mismatch"); \
        return static_cast<outtype>(parameterPtr->parameter.valfrom); \
    } \
    void ShaderParameters::set##ptype##GlobalParameter(const String &name, const intype &value) \
    { \
        lite3d_shader_parameter *parameterPtr = getGlobalParameter(name, enums, true); \
        parameterPtr->parameter.valto = value; \
    } \
    outtype ShaderParameters::get##ptype##GlobalParameter(const String &name) \
    { \
        lite3d_shader_parameter *parameterPtr = getGlobalParameter(name, enums, false); \
        if (parameterPtr->type != enums) \
            LITE3D_THROW("Global parameter " << name << " type mismatch"); \
        return static_cast<outtype>(parameterPtr->parameter.valfrom); \
    }

namespace lite3dpp
{
    ShaderParameters::ShaderParameters(lite3d_shader_parameters *data) : 
        mData(data) 
    {
        SDL_assert(mData);
    }

    lite3d_shader_parameter *ShaderParameters::getParameter(const String &name, 
        uint8_t type, bool isGlobal, bool createIfNotExist)
    {
        if (isGlobal)
        {
            auto param = getGlobalParameter(name, type, createIfNotExist);
            if (mUsedGlobalParameters.count(name) == 0)
            {
                mUsedGlobalParameters.emplace(name);
                lite3d_shader_parameters_add(mData, param);
            }

            return param;
        }

        ParametersStore::iterator it;
        if ((it = mParameters.find(name)) == mParameters.end())
        {
            if (!createIfNotExist)
            {
                LITE3D_THROW("Parameter " << name << " is not found..");
            }

            auto result = mParameters.emplace(name, lite3d_shader_parameter());
            it = result.first;
            
            lite3d_shader_parameter_init(&it->second);
            name.copy(it->second.name, sizeof(it->second.name)-1);
            it->second.type = type;
            lite3d_shader_parameters_add(mData, &it->second);
        }

        return &it->second;
    }

    bool ShaderParameters::hasParameter(const String &name, bool isGlobal) const
    {
        if(isGlobal)
        {
            return mUsedGlobalParameters.count(name) > 0;
        }

        return mParameters.count(name) > 0;
    }
    
    lite3d_shader_parameter *ShaderParameters::getGlobalParameter(const String &name, 
        uint8_t type, bool createIfNotExist)
    {
        ParametersStore::iterator it;
        /* built-in global parameters */
        if(name == "projectionMatrix")
            return &lite3d_shader_global_parameters()->projectionMatrix;
        else if(name == "viewMatrix")
            return &lite3d_shader_global_parameters()->viewMatrix;
        else if(name == "modelMatrix")
            return &lite3d_shader_global_parameters()->modelMatrix;
        else if(name == "normalMatrix")
            return &lite3d_shader_global_parameters()->normalMatrix;
        else if(name == "screenMatrix")
            return &lite3d_shader_global_parameters()->screenMatrix;
        else if(name == "projViewMatrix")
            return &lite3d_shader_global_parameters()->projViewMatrix;

        /* User global parameters */
        if ((it = mGlobalParameters.find(name)) == mGlobalParameters.end())
        {
            if (!createIfNotExist)
            {
                LITE3D_THROW("Global parameter " << name << " is not found..");
            }

            auto result = mGlobalParameters.try_emplace(name, lite3d_shader_parameter());
            it = result.first;
            
            lite3d_shader_parameter_init(&it->second);
            name.copy(it->second.name, sizeof(it->second.name)-1);
            it->second.type = type;
        }

        return &it->second;
    }

    void ShaderParameters::clear()
    {
        SDL_assert(mData);
        lite3d_shader_parameters_remove_all(mData);
    }

    LITE3D_IMPLEMENT_SHADER_PARAMETER(Float, float, float, LITE3D_SHADER_PARAMETER_FLOAT, valfloat, valfloat)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Int, int32_t, int32_t, LITE3D_SHADER_PARAMETER_INT, valint, valint)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(UInt, uint32_t, uint32_t, LITE3D_SHADER_PARAMETER_UINT, valuint, valuint)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Floatv3, kmVec3, kmVec3, LITE3D_SHADER_PARAMETER_FLOATV3, valvec3, valvec3)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Floatv4, kmVec4, kmVec4, LITE3D_SHADER_PARAMETER_FLOATV4, valvec4, valvec4)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Floatm3, kmMat3, kmMat3, LITE3D_SHADER_PARAMETER_FLOATM3, valmat3, valmat3)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Floatm4, kmMat4, kmMat4, LITE3D_SHADER_PARAMETER_FLOATM4, valmat4, valmat4)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(Sampler, Texture, Texture *, LITE3D_SHADER_PARAMETER_SAMPLER, texture, texture->userdata)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(SSBO, SSBO, SSBO *, LITE3D_SHADER_PARAMETER_SSBO, vbo, vbo->userdata)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(UBO, UBO, UBO *, LITE3D_SHADER_PARAMETER_UBO, vbo, vbo->userdata)
    LITE3D_IMPLEMENT_SHADER_PARAMETER(ImageStore, Texture, Texture *, LITE3D_SHADER_PARAMETER_IMAGE_STORE, texture, texture->userdata)
}

