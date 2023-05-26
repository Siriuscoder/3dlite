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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_buffers_manip.h>
#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_material.h>

lite3dpp::Material::MaterialParameters lite3dpp::Material::mGlobalParameters;

#define LITE3D_IMPLEMENT_MAT_PARAMETER(ptype, intype, outtype, enums, valto, valfrom) \
    void Material::set##ptype##Parameter(uint16_t pass, const String &name, const intype &value, bool isGlobal) \
    { \
        lite3d_material_pass *passPtr = NULL; \
        lite3d_shader_parameter *parameterPtr; \
        if(pass > 0) \
        { \
            passPtr = lite3d_material_get_pass(&mMaterial, pass); \
            if(!passPtr) \
                LITE3D_THROW("Material \"" << getName() << "\" pass not found.."); \
        } \
        parameterPtr = getParameter(name, enums, passPtr, isGlobal); \
        SDL_assert_release(parameterPtr); \
        parameterPtr->parameter.valto = value; \
        addParameter(passPtr, parameterPtr); \
    }\
    outtype Material::get##ptype##Parameter(const String &name) const \
    { \
        if(mGlobalParamNames.count(name) > 0) \
            return get##ptype##ParameterFromMap(name, getName(), mGlobalParameters); \
        return get##ptype##ParameterFromMap(name, getName(), mMaterialParameters); \
    }\
    void Material::set##ptype##GlobalParameter(const String &name, const intype &value) \
    { \
        lite3d_shader_parameter *parameterPtr; \
        parameterPtr = getGlobalParameter(name, enums); \
        SDL_assert_release(parameterPtr); \
        parameterPtr->parameter.valto = value;\
    }\
    outtype Material::get##ptype##GlobalParameter(const String &name)\
    {\
        return get##ptype##ParameterFromMap(name, "...", mGlobalParameters);\
    }\
    outtype Material::get##ptype##ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)\
    {\
        MaterialParameters::const_iterator it;\
        if((it = params.find(name)) == params.end()) \
        {\
            if(std::get<1>(it->second).type != enums)\
                LITE3D_THROW("Material \"" << matName << "\" parameter " << name << " type mismatch");\
            return static_cast<outtype>(std::get<1>(it->second).parameter.valfrom);\
        }\
        LITE3D_THROW("Material \"" << matName << "\" parameter " << name << " not found..");\
    }

namespace lite3dpp
{
    Material::Material(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::MATERIAL)
    {}

    Material::~Material()
    {}

    void Material::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        lite3d_material_init(&mMaterial);
        for(const ConfigurationReader &passJson : helper.getObjects(L"Passes"))
        {
            uint16_t passNo = passJson.getInt(L"Pass");
            addPass(passNo);

            ConfigurationReader programJson = passJson.getObject(L"Program");
            setPassProgram(passNo, mMain->getResourceManager()->queryResource<ShaderProgram>(programJson.getString(L"Name"),
                programJson.getString(L"Path")));
            
            lite3dpp::String bMode = passJson.getUpperString(L"BlendingMode", "TRADITIONAL");
            if (bMode == "TRADITIONAL")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_TRADITIONAL);
            }
            else if (bMode == "ADDITIVE")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_ADDITIVE);
            }
            else if (bMode == "TRADITIONALWITHALPHA")
            {
                setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                    LITE3D_BLENDING_MODE_TRADITIONAL_WITH_ALPHA_BLEND);
            }
            else
            {
                LITE3D_THROW(getName() << " unsupported BlendingMode");
            }

            parseParameteres(passJson, passNo);
        }

        mMaterial.userdata = this;
        parseParameteres(helper, 0);
    }

    void Material::parseParameteres(const ConfigurationReader &passJson, uint16_t passNo)
    {
        for(const ConfigurationReader &uniformParamJson : passJson.getObjects(L"Uniforms"))
        {
            String paramName = uniformParamJson.getString(L"Name");
            /* check for built-in parameters */
            if(paramName == "projectionMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->projectionMatrix);
            else if(paramName == "viewMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->viewMatrix);
            else if(paramName == "modelMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->modelMatrix);
            else if(paramName == "normalMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->normalMatrix);
            else if(paramName == "screenMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->screenMatrix);
            else if(paramName == "projViewMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->projViewMatrix);
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
                        *mMain->getResourceManager()->queryResource<TextureImage>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                else if(paramType == "samplerBuffer")
                    setSamplerParameter(passNo, paramName, 
                        *mMain->getResourceManager()->queryResource<TextureBuffer>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
                else if(paramType == "SSBO")
                    setSSBOParameter(passNo, paramName, 
                        *mMain->getResourceManager()->queryResource<SSBO>(uniformParamJson.getString(L"SSBOName"),
                        uniformParamJson.getString(L"SSBOPath")), scope == "global");
                else if(paramType == "UBO")
                    setUBOParameter(passNo, paramName, 
                        *mMain->getResourceManager()->queryResource<UBO>(uniformParamJson.getString(L"UBOName"),
                        uniformParamJson.getString(L"UBOPath")), scope == "global");
            }
        }
    }

    void Material::unloadImpl()
    {
        lite3d_material_purge(&mMaterial);
        mMaterialParameters.clear();
        mPasses.clear();
    }
    
    void Material::addPass(uint16_t passNo)
    {
        lite3d_material_pass *passPtr;
        
        if(!(passPtr = lite3d_material_add_pass(&mMaterial, passNo)))
            LITE3D_THROW("Material \"" << getName() << "\" add pass failed..");
        
        /* add local parameters to new pass */
        for(auto &parameter : mMaterialParameters)
        {
            /* meant that this parameter must used in all passes */
            if(std::get<0>(parameter.second) == NULL)
            {
                lite3d_material_pass_add_parameter(passPtr, &std::get<1>(parameter.second));
            }
        }
        
        /* add global parameters to new pass */
        for(auto &nameParam : mGlobalParamNames)
        {
            /* meant that this parameter must used in all passes */
            if(nameParam.second == NULL)
            {
                lite3d_material_pass_add_parameter(passPtr, &std::get<1>(mGlobalParameters[nameParam.first]));
            }
        }
        
        mPasses.insert(std::make_pair(mMaterial.passesSize, passPtr));
    }
    
    void Material::removePass(uint16_t pass)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass not found..");

        /* remove all parameters associated with this pass */
        MaterialParameters::iterator it = mMaterialParameters.begin();
        for(; it != mMaterialParameters.end();)
        {
            if(std::get<0>(it->second) == passPtr)
            {
                it = mMaterialParameters.erase(it);
            }
            else
                it++;
        }

        lite3d_material_remove_pass(&mMaterial, pass);
        mPasses.erase(pass);
    }
    
    void Material::setPassBlendMode(uint16_t pass, bool blendEnable, uint8_t mode)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass not found..");
        
        passPtr->blending = blendEnable ? LITE3D_TRUE : LITE3D_FALSE;
        passPtr->blendingMode = mode;
    }
    
    void Material::setPassProgram(uint16_t pass, ShaderProgram *program)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass not found..");
        
        passPtr->program = program->getPtr();
    }
    
    ShaderProgram *Material::getPassProgram(uint16_t pass) const
    {
        const lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            LITE3D_THROW("Material \"" << getName() << "\" pass not found..");
        if(!passPtr->program)
            LITE3D_THROW("Material \"" << getName() << "\" program not specified..");
        
        return static_cast<ShaderProgram *>(passPtr->program->userdata);
    }

    lite3d_shader_parameter *Material::getParameter(const String &name, 
        uint8_t type, lite3d_material_pass *passPtr, bool isGlobal)
    {
        MaterialParameters::iterator it;

        if(isGlobal)
        {
            mGlobalParamNames.insert(std::make_pair(name, passPtr));
            return getGlobalParameter(name, type);
        }

        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            auto result = mMaterialParameters.insert(std::make_pair(name, std::make_tuple(passPtr, lite3d_shader_parameter())));
            it = result.first;
            
            lite3d_shader_parameter_init(&std::get<1>(it->second));
            
            std::get<0>(it->second) = passPtr;
            name.copy(std::get<1>(it->second).name, sizeof(std::get<1>(it->second).name)-1);
            std::get<1>(it->second).type = type;
        }

        return &std::get<1>(it->second);
    }
    
    lite3d_shader_parameter *Material::getGlobalParameter(const String &name, 
        uint8_t type)
    {
        MaterialParameters::iterator it;
        lite3d_material_pass *passPtr = NULL;

        if((it = mGlobalParameters.find(name)) == mGlobalParameters.end())
        {
            auto result = mGlobalParameters.insert(std::make_pair(name, 
                std::make_tuple(passPtr, lite3d_shader_parameter())));
            it = result.first;
            
            lite3d_shader_parameter_init(&std::get<1>(it->second));
            
            name.copy(std::get<1>(it->second).name, sizeof(std::get<1>(it->second).name)-1);
            std::get<1>(it->second).type = type;
        }

        return &std::get<1>(it->second);
    }
    

    void Material::addParameter(lite3d_material_pass *passPtr, lite3d_shader_parameter *parameterPtr)
    {
        if(passPtr)
            lite3d_material_pass_add_parameter(passPtr, parameterPtr);
        else
        {
            for(Passes::value_type &passEntry : mPasses)
                lite3d_material_pass_add_parameter(passEntry.second, parameterPtr);
        }
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

