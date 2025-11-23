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

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_compute_shader.h>

namespace lite3dpp
{
    ComputeShader::ComputeShader(const String &name, const String &path, Main &main) : 
        ConfigurableResource(name, path, main, AbstractResource::SHADER_PROGRAM),
        mShaderParameters(&mShaderParametersData)
    {
        lite3d_shader_parameters_init(&mShaderParametersData);
    }

    void ComputeShader::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        ConfigurationReader programJson = helper.getObject(L"Program");
        mProgram = getMain().getResourceManager().queryResource<ShaderProgram>(programJson.getString(L"Name"),
                programJson.getString(L"Path"));

        for(const ConfigurationReader &uniformParamJson : helper.getObjects(L"Uniforms"))
        {
            String paramName = uniformParamJson.getString(L"Name");
            /* check for built-in parameters */
            if (paramName == "projectionMatrix" ||
                paramName == "viewMatrix" ||
                paramName == "modelMatrix" ||
                paramName == "normalMatrix" ||
                paramName == "screenMatrix" ||
                paramName == "projViewMatrix")
            {
                mShaderParameters.getParameter(paramName, 0, true, true);
            }
            else
            /* user parameters */
            {
                String paramType = uniformParamJson.getString(L"Type");
                String scope = uniformParamJson.getString(L"Scope", "local");
                if (paramType == "float")
                    mShaderParameters.setFloatParameter(paramName, uniformParamJson.getDouble(L"Value"), scope == "global");
                else if (paramType == "v3")
                    mShaderParameters.setFloatv3Parameter(paramName, uniformParamJson.getVec3(L"Value"), scope == "global");
                else if (paramType == "v4")
                    mShaderParameters.setFloatv4Parameter(paramName, uniformParamJson.getVec4(L"Value"), scope == "global");
                else if (paramType == "int")
                    mShaderParameters.setIntParameter(paramName, uniformParamJson.getInt(L"Value"), scope == "global");
                else if (paramType == "uint")
                    mShaderParameters.setUIntParameter(paramName, static_cast<uint32_t>(uniformParamJson.getInt(L"Value")), scope == "global");
                else if (paramType == "m3")
                {
                    kmMat3 mat3;
                    kmMat3Identity(&mat3);
                    mShaderParameters.setFloatm3Parameter(paramName, uniformParamJson.getMat3(L"Value", mat3), scope == "global");
                }
                else if (paramType == "m4")
                {
                    kmMat4 mat4;
                    kmMat4Identity(&mat4);
                    mShaderParameters.setFloatm4Parameter(paramName, uniformParamJson.getMat4(L"Value", mat4), scope == "global");
                }
                else if (paramType == "sampler")
                    mShaderParameters.setSamplerParameter(paramName, 
                        *getMain().getResourceManager().queryResource<TextureImage>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                else if (paramType == "SSBO")
                    mShaderParameters.setSSBOParameter(paramName, 
                        *getMain().getResourceManager().queryResource<SSBO>(uniformParamJson.getString(L"SSBOName"),
                        uniformParamJson.getString(L"SSBOPath")), scope == "global");
                else if (paramType == "UBO")
                    mShaderParameters.setUBOParameter(paramName, 
                        *getMain().getResourceManager().queryResource<UBO>(uniformParamJson.getString(L"UBOName"),
                        uniformParamJson.getString(L"UBOPath")), scope == "global");
                else if (paramType == "imageStore")
                {
                    mShaderParameters.setImageStoreParameter(paramName, 
                        *getMain().getResourceManager().queryResource<TextureImage>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                    auto parameter = mShaderParameters.getParameter(paramName, LITE3D_SHADER_PARAMETER_IMAGE_STORE, 
                        scope == "global", false);

                    // Set direction of parameter based on json value
                    // "inout" sets direction to inout, "in" sets direction to input, "out" sets direction to output
                    auto direction = uniformParamJson.getString(L"Direction", "inout");
                    parameter->direction = direction == "inout" ? LITE3D_SHADER_PARAMETER_DIRECTION_INOUT :
                        (direction == "in" ? LITE3D_SHADER_PARAMETER_DIRECTION_INPUT : LITE3D_SHADER_PARAMETER_DIRECTION_OUTPUT);

                    // Set layer of parameter based on json value
                    // -1 means no layer specified
                    parameter->imageLayer = uniformParamJson.getInt(L"Layer", -1);

                    // Set mip level of parameter based on json value
                    // 0 means highest mip level
                    parameter->imageMipLevel = uniformParamJson.getInt(L"MipLevel", 0);
                }
                else
                    LITE3D_THROW("ComputeShader \"" << getName() << "\": unknown parameter type \"" << paramType << "\"");
            }
        }
    }

    void ComputeShader::unloadImpl()
    {
        mShaderParameters.clear();
    }

    void ComputeShader::dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        if (!mProgram)
        {
            LITE3D_THROW("Program \"" << getName() << "\"is not loaded");
        }

        /* bind current shander first */
        lite3d_shader_program_bind(mProgram->getPtr());
        /* set up uniforms if shader changed */
        lite3d_shader_program_apply_parameters(mProgram->getPtr(), &mShaderParametersData, LITE3D_TRUE);
        lite3d_shader_program_compute_dispatch(mProgram->getPtr(), numGroupsX, numGroupsY, numGroupsZ);
    }

    void ComputeShader::dispatchSync(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        if (!mProgram)
        {
            LITE3D_THROW("Program \"" << getName() << "\"is not loaded");
        }

        /* bind current shander first */
        lite3d_shader_program_bind(mProgram->getPtr());
        /* set up uniforms if shader changed */
        lite3d_shader_program_apply_parameters(mProgram->getPtr(), &mShaderParametersData, LITE3D_TRUE);
        lite3d_shader_program_compute_dispatch_sync(mProgram->getPtr(), numGroupsX, numGroupsY, numGroupsZ);
    }

    void ComputeShader::addLocalDefinition(const String &name, const String &value)
    {
        if (mProgram)
        {
            mProgram->addLocalDefinition(name, value);
        }
    }
}
