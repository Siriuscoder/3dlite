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
            
            lite3dpp::String bMode = passJson.getString(L"BlendingMode", "traditional");
            setPassBlendMode(passNo, passJson.getBool(L"Blending", false), 
                bMode == "Traditional" ? LITE3D_BLENDING_MODE_TRADITIONAL : 
                LITE3D_BLENDING_MODE_TRADITIONAL_WITH_ALPHA_BLEND);

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
            else if(paramName == "modelviewMatrix")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->modelviewMatrix);
            else if(paramName == "ambientLight")
                addParameter(lite3d_material_get_pass(&mMaterial, passNo),
                    &lite3d_shader_global_parameters()->ambientLight);
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
                else if(paramType == "sampler")
                    setSamplerTextureParameter(passNo, paramName, 
                        mMain->getResourceManager()->queryResource<Texture>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")), scope == "global");
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
            throw std::runtime_error(String("Material \"") + getName() + "\" add pass failed..");
        
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
            throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");

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
            throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        
        passPtr->blending = blendEnable ? LITE3D_TRUE : LITE3D_FALSE;
        passPtr->blendingMode = mode;
    }
    
    void Material::setPassProgram(uint16_t pass, ShaderProgram *program)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        
        passPtr->program = program->getPtr();
    }
    
    ShaderProgram *Material::getPassProgram(uint16_t pass) const
    {
        const lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        if(!passPtr->program)
            throw std::runtime_error(String("Material \"") + getName() + "\" program not specified..");
        
        return static_cast<ShaderProgram *>(passPtr->program->userdata);
    }
    
    void Material::setFloatParameter(uint16_t pass, const String &name, float value, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOAT, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valfloat = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatv3Parameter(uint16_t pass, const String &name, const kmVec3 &value, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATV3, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec3 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatv4Parameter(uint16_t pass, const String &name, const kmVec4 &value, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATV4, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec4 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatm3Parameter(uint16_t pass, const String &name, const kmMat3 &value, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATM3, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat3 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatm4Parameter(uint16_t pass, const String &name, const kmMat4 &value, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATM4, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat4 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setSamplerTextureParameter(uint16_t pass, const String &name, Texture *texture, bool isGlobal)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        SDL_assert_release(texture);
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_SAMPLER, passPtr, isGlobal);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valsampler.texture = texture->getPtr();
        
        addParameter(passPtr, parameterPtr);
    }

    float Material::getFloatParameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getFloatParameterFromMap(name, getName(), mGlobalParameters);
        
        return getFloatParameterFromMap(name, getName(), mMaterialParameters);
    }

    kmVec3 Material::getFloatv3Parameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getFloatv3ParameterFromMap(name, getName(), mGlobalParameters);
        
        return getFloatv3ParameterFromMap(name, getName(), mMaterialParameters);
    }

    kmVec4 Material::getFloatv4Parameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getFloatv4ParameterFromMap(name, getName(), mGlobalParameters);
        
        return getFloatv4ParameterFromMap(name, getName(), mMaterialParameters);
    }

    kmMat3 Material::getFloatm3Parameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getFloatm3ParameterFromMap(name, getName(), mGlobalParameters);
        
        return getFloatm3ParameterFromMap(name, getName(), mMaterialParameters);
    }

    kmMat4 Material::getFloatm4Parameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getFloatm4ParameterFromMap(name, getName(), mGlobalParameters);
        
        return getFloatm4ParameterFromMap(name, getName(), mMaterialParameters);
    }

    Texture *Material::getSamplerTextureParameter(const String &name) const
    {
        if(mGlobalParamNames.count(name) > 0)
            return getSamplerTextureParameterFromMap(name, getName(), mGlobalParameters);
        
        return getSamplerTextureParameterFromMap(name, getName(), mMaterialParameters);
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
    
    void Material::setFloatGlobalParameter(const String &name, float value)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_FLOAT);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valfloat = value;
    }
    
    void Material::setFloatv3GlobalParameter(const String &name, const kmVec3 &value)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_FLOATV3);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec3 = value;        
    }
    
    void Material::setFloatv4GlobalParameter(const String &name, const kmVec4 &value)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_FLOATV4);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec4 = value;
    }
    
    void Material::setFloatm3GlobalParameter(const String &name, const kmMat3 &value)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_FLOATM3);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat3 = value;      
    }
    
    void Material::setFloatm4GlobalParameter(const String &name, const kmMat4 &value)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_FLOATM4);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat4 = value;
    }
    
    void Material::setSamplerTextureGlobalParameter(const String &name, Texture *texture)
    {
        lite3d_shader_parameter *parameterPtr;

        parameterPtr = getGlobalParameter(name, LITE3D_SHADER_PARAMETER_SAMPLER);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valsampler.texture = texture->getPtr();
    }
        
    float Material::getFloatGlobalParameter(const String &name)
    {
        return getFloatParameterFromMap(name, "...", mGlobalParameters);
    }
    
    kmVec3 Material::getFloatv3GlobalParameter(const String &name)
    {
        return getFloatv3ParameterFromMap(name, "...", mGlobalParameters);        
    }
    
    kmVec4 Material::getFloatv4GlobalParameter(const String &name)
    {
        return getFloatv4ParameterFromMap(name, "...", mGlobalParameters);        
    }
    
    kmMat3 Material::getFloatm3GlobalParameter(const String &name)
    {
        return getFloatm3ParameterFromMap(name, "...", mGlobalParameters);        
    }
    
    kmMat4 Material::getFloatm4GlobalParameter(const String &name)
    {
        return getFloatm4ParameterFromMap(name, "...", mGlobalParameters);        
    }
    
    Texture *Material::getSamplerTextureGlobalParameter(const String &name)
    {
        return getSamplerTextureParameterFromMap(name, "...", mGlobalParameters);        
    }

    float Material::getFloatParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOAT)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valfloat;
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }

    kmVec3 Material::getFloatv3ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATV3)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valvec3;
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }

    kmVec4 Material::getFloatv4ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATV4)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valvec4;
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }

    kmMat3 Material::getFloatm3ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATM3)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valmat3;
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }

    kmMat4 Material::getFloatm4ParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATM4)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valmat4;
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }

    Texture *Material::getSamplerTextureParameterFromMap(const String &name, const String &matName, const MaterialParameters &params)
    {
        MaterialParameters::const_iterator it;
        if((it = params.find(name)) == params.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_SAMPLER)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " type mismatch");

            if(!std::get<1>(it->second).parameter.valsampler.texture || !std::get<1>(it->second).parameter.valsampler.texture->userdata)
                throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not set");

            return static_cast<Texture *>(std::get<1>(it->second).parameter.valsampler.texture->userdata);
        }

        throw std::runtime_error(String("Material \"") + matName + "\" parameter " + name + " not found..");
    }
}

