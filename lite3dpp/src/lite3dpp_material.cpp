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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_buffers_manip.h>
#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_material.h>

lite3dpp::Material::PassParameters lite3dpp::Material::mGlobalParameters;

#define LITE3D_IMPLEMENT_MAT_PARAMETER(ptype, intype, outtype, enums, valto, valfrom) \
    void Material::set##ptype##Parameter(uint16_t passNo, const String &name, const intype &value, bool isGlobal) \
    { \
        lite3d_shader_parameter *parameterPtr = getParameter(name, enums, passNo, isGlobal, true); \
        parameterPtr->parameter.valto = value; \
    } \
    outtype Material::get##ptype##Parameter(const String &name, uint16_t passNo) \
    { \
        lite3d_shader_parameter *parameterPtr = getParameter(name, enums, passNo, false, false); \
        if (parameterPtr->type != enums) \
            LITE3D_THROW("Material \"" << getName() << "\" parameter " << name << " type mismatch"); \
        return static_cast<outtype>(parameterPtr->parameter.valfrom); \
    } \
    void Material::set##ptype##GlobalParameter(const String &name, const intype &value) \
    { \
        lite3d_shader_parameter *parameterPtr = getGlobalParameter(name, enums, true); \
        parameterPtr->parameter.valto = value; \
    } \
    outtype Material::get##ptype##GlobalParameter(const String &name) \
    { \
        lite3d_shader_parameter *parameterPtr = getGlobalParameter(name, enums, false); \
        if (parameterPtr->type != enums) \
            LITE3D_THROW("Material global parameter " << name << " type mismatch"); \
        return static_cast<outtype>(parameterPtr->parameter.valfrom); \
    }

namespace lite3dpp
{
    Material::Material(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MATERIAL)
    {}

    void Material::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_material_init(&mMaterial);
        for(const ConfigurationReader &passJson : helper.getObjects(L"Passes"))
        {
            uint16_t passNo = passJson.getInt(L"Pass");
            addPass(passNo);

            ConfigurationReader programJson = passJson.getObject(L"Program");
            setPassProgram(passNo, getMain().getResourceManager()->queryResource<ShaderProgram>(programJson.getString(L"Name"),
                programJson.getString(L"Path")));
            
            String bMode = passJson.getUpperString(L"BlendingMode", "RGB_LINEAR_SOURCE_ALPHA");
            if (bMode == "RGB_LINEAR_SOURCE_ALPHA")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_RGB_LINEAR_SOURCE_ALPHA);
            }
            else if (bMode == "RGB_ADDITIVE")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_RGB_ADDITIVE);
            }
            else if (bMode == "RGBA_LINEAR_SOURCE_ALPHA")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_RGBA_LINEAR_SOURCE_ALPHA);
            }
            else
            {
                LITE3D_THROW(getName() << " unsupported BlendingMode");
            }

            setDoubleSided(passNo, passJson.getBool(L"DoubleSided", false));
            String polygonMode = passJson.getUpperString(L"PolygonMode", "FILL");
            if (polygonMode == "FILL")
            {
                setPolygonMode(passNo, PolygonMode::PolygonFill);
            }
            else if (polygonMode == "LINE")
            {
                setPolygonMode(passNo, PolygonMode::PolygonLine);
            }
            else if (polygonMode == "POINT")
            {
                setPolygonMode(passNo, PolygonMode::PolygonPoint);
            }
            else
            {
                LITE3D_THROW(getName() << " unsupported PolygonMode");
            }

            parseParameteres(passJson, passNo);
            parseParameteres(helper, passNo);
        }

        mMaterial.userdata = this;
        
    }

    void Material::parseParameteres(const ConfigurationReader &passJson, uint16_t passNo)
    {
        for(const ConfigurationReader &uniformParamJson : passJson.getObjects(L"Uniforms"))
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
                getParameter(paramName, 0, passNo, true, true);
            }
            else
            /* user parameters */
            {
                String paramType = uniformParamJson.getString(L"Type");
                String scope = uniformParamJson.getString(L"Scope", "local");
                if(paramType == "float")
                    setFloatParameter(passNo, paramName, uniformParamJson.getDouble(L"Value"), scope == "global");
                else if(paramType == "v3")
                    setFloatv3Parameter(passNo, paramName, uniformParamJson.getVec3(L"Value"), scope == "global");
                else if(paramType == "v4")
                    setFloatv4Parameter(passNo, paramName, uniformParamJson.getVec4(L"Value"), scope == "global");
                else if(paramType == "int")
                    setIntParameter(passNo, paramName, uniformParamJson.getInt(L"Value"), scope == "global");
                else if(paramType == "m3")
                {
                    kmMat3 mat3;
                    kmMat3Identity(&mat3);
                    setFloatm3Parameter(passNo, paramName, uniformParamJson.getMat3(L"Value", mat3), scope == "global");
                }
                else if(paramType == "m4")
                {
                    kmMat4 mat4;
                    kmMat4Identity(&mat4);
                    setFloatm4Parameter(passNo, paramName, uniformParamJson.getMat4(L"Value", mat4), scope == "global");
                }
                else if(paramType == "sampler")
                    setSamplerParameter(passNo, paramName, 
                        *getMain().getResourceManager()->queryResource<TextureImage>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                else if(paramType == "samplerBuffer")
                    setSamplerParameter(passNo, paramName, 
                        *getMain().getResourceManager()->queryResource<TextureBuffer>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                else if(paramType == "SSBO")
                    setSSBOParameter(passNo, paramName, 
                        *getMain().getResourceManager()->queryResource<SSBO>(uniformParamJson.getString(L"SSBOName"),
                        uniformParamJson.getString(L"SSBOPath")), scope == "global");
                else if(paramType == "UBO")
                    setUBOParameter(passNo, paramName, 
                        *getMain().getResourceManager()->queryResource<UBO>(uniformParamJson.getString(L"UBOName"),
                        uniformParamJson.getString(L"UBOPath")), scope == "global");
            }
        }
    }

    void Material::unloadImpl()
    {
        lite3d_material_purge(&mMaterial);
        mPasses.clear();
    }
    
    void Material::addPass(uint16_t passNo)
    {
        lite3d_material_pass *passPtr;
        
        if (passNo > LITE3D_PASSNO_MAX)
            LITE3D_THROW("Material \"" << getName() << "\" maximum valid passNo is " << LITE3D_PASSNO_MAX);
        
        if(!(passPtr = lite3d_material_add_pass(&mMaterial, passNo)))
            LITE3D_THROW("Material \"" << getName() << "\" add pass failed..");
        
        mPasses.try_emplace(passNo, PassParameters(), GlobalPassParameters());
    }
    
    void Material::removePass(uint16_t pass)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");

        lite3d_material_remove_pass(&mMaterial, pass);
        mPasses.erase(pass);
    }
    
    void Material::setPassBlendMode(uint16_t pass, bool blendEnable, uint8_t mode)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");
        
        passPtr->blending = blendEnable ? LITE3D_TRUE : LITE3D_FALSE;
        passPtr->blendingMode = mode;
    }
    
    void Material::setPassProgram(uint16_t pass, ShaderProgram *program)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");
        
        passPtr->program = program->getPtr();
    }

    void Material::setPolygonMode(uint16_t pass, PolygonMode mode)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");

        passPtr->polygonMode = static_cast<uint8_t>(mode);
    }

    void Material::setDoubleSided(uint16_t pass, bool flag)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");

        passPtr->doubleSided = flag ? LITE3D_TRUE : LITE3D_FALSE;
    }

    ShaderProgram *Material::getPassProgram(uint16_t pass) const
    {
        const lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");
        if(!passPtr->program)
            LITE3D_THROW("Material \"" << getName() << "\" program is not specified..");
        
        return static_cast<ShaderProgram *>(passPtr->program->userdata);
    }

    lite3d_shader_parameter *Material::getParameter(const String &name, 
        uint8_t type, uint16_t passNo, bool isGlobal, bool createIfNotExist)
    {
        auto passIt = mPasses.find(passNo);
        if (passIt == mPasses.end())
        {  
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");
        }

        auto passPtr = lite3d_material_get_pass(&mMaterial, passNo);
        if (!passPtr)
        {  
            LITE3D_THROW("Material \"" << getName() << "\" pass is not found..");
        }

        if (isGlobal)
        {
            auto param = getGlobalParameter(name, type, createIfNotExist);
            if (std::get<1>(passIt->second).count(name) == 0)
            {
                std::get<1>(passIt->second).emplace(name);
                lite3d_material_pass_add_parameter(passPtr, param);
            }

            return param;
        }

        auto &passParams = std::get<0>(passIt->second);
        PassParameters::iterator it;
        if ((it = passParams.find(name)) == passParams.end())
        {
            if (!createIfNotExist)
            {
                LITE3D_THROW("Material \"" << getName() << "\" parameter " << name << " is not found..");
            }

            auto result = passParams.emplace(name, lite3d_shader_parameter());
            it = result.first;
            
            lite3d_shader_parameter_init(&it->second);
            name.copy(it->second.name, sizeof(it->second.name)-1);
            it->second.type = type;
            lite3d_material_pass_add_parameter(passPtr, &it->second);
        }

        return &it->second;
    }

    bool Material::hasParameter(const String &name, uint16_t passNo, bool isGlobal) const
    {
        auto passIt = mPasses.find(passNo);
        if (passIt == mPasses.end())
        {
            return false;
        }

        if(isGlobal)
        {
            return std::get<1>(passIt->second).count(name) > 0;
        }

        return std::get<0>(passIt->second).count(name) > 0;
    }
    
    lite3d_shader_parameter *Material::getGlobalParameter(const String &name, 
        uint8_t type, bool createIfNotExist)
    {
        PassParameters::iterator it;
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
        if((it = mGlobalParameters.find(name)) == mGlobalParameters.end())
        {
            if (!createIfNotExist)
            {
                LITE3D_THROW("Material global parameter " << name << " is not found..");
            }

            auto result = mGlobalParameters.try_emplace(name, lite3d_shader_parameter());
            it = result.first;
            
            lite3d_shader_parameter_init(&it->second);
            name.copy(it->second.name, sizeof(it->second.name)-1);
            it->second.type = type;
        }

        return &it->second;
    }
    
    LITE3D_IMPLEMENT_MAT_PARAMETER(Float, float, float, LITE3D_SHADER_PARAMETER_FLOAT, valfloat, valfloat)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Int, int32_t, int32_t, LITE3D_SHADER_PARAMETER_INT, valint, valint)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Floatv3, kmVec3, kmVec3, LITE3D_SHADER_PARAMETER_FLOATV3, valvec3, valvec3)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Floatv4, kmVec4, kmVec4, LITE3D_SHADER_PARAMETER_FLOATV4, valvec4, valvec4)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Floatm3, kmMat3, kmMat3, LITE3D_SHADER_PARAMETER_FLOATM3, valmat3, valmat3)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Floatm4, kmMat4, kmMat4, LITE3D_SHADER_PARAMETER_FLOATM4, valmat4, valmat4)
    LITE3D_IMPLEMENT_MAT_PARAMETER(Sampler, Texture, Texture *, LITE3D_SHADER_PARAMETER_SAMPLER, texture, texture->userdata)
    LITE3D_IMPLEMENT_MAT_PARAMETER(SSBO, SSBO, SSBO *, LITE3D_SHADER_PARAMETER_SSBO, vbo, vbo->userdata)
    LITE3D_IMPLEMENT_MAT_PARAMETER(UBO, UBO, UBO *, LITE3D_SHADER_PARAMETER_UBO, vbo, vbo->userdata)
}

