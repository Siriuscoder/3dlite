/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_material.h>

namespace lite3dpp
{
    Material::Material(const String &name, 
        const String &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::MATERIAL)
    {}

    Material::~Material()
    {}

    void Material::loadFromJsonImpl(const JsonHelper &helper)
    {
        lite3d_material_init(&mMaterial);
        for(const JsonHelper &passJson : helper.getObjects(L"Passes"))
        {
            uint16_t passNo = passJson.getInt(L"Pass");
            addPass(passNo);

            JsonHelper programJson = passJson.getObject(L"Program");
            setPassProgram(passNo, mMain->getResourceManager()->queryResource<ShaderProgram>(programJson.getString(L"Name"),
                programJson.getString(L"Path")));

            parseParameteres(passJson, passNo);
        }

        parseParameteres(helper, 0);
    }

    void Material::parseParameteres(const JsonHelper &passJson, uint16_t passNo)
    {
        for(const JsonHelper &uniformParamJson : passJson.getObjects(L"Uniforms"))
        {
            String paramName = uniformParamJson.getString(L"Name");
            /* check for global parameters */
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
                if(paramType == "float")
                    setFloatParameter(passNo, paramName, uniformParamJson.getDouble(L"Value"));
                if(paramType == "v3")
                    setFloatv3Parameter(passNo, paramName, uniformParamJson.getVec3(L"Value"));
                if(paramType == "v4")
                    setFloatv4Parameter(passNo, paramName, uniformParamJson.getVec4(L"Value"));
                if(paramType == "sampler")
                    setSamplerTextureParameter(passNo, paramName, 
                        mMain->getResourceManager()->queryResource<Texture>(uniformParamJson.getString(L"TextureName"),
                        uniformParamJson.getString(L"TexturePath")));
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
        
        for(MaterialParameters::value_type &parameter : mMaterialParameters)
        {
            /* meant that this parameter must used in all passes */
            if(std::get<0>(parameter.second) == NULL)
            {
                lite3d_material_pass_add_parameter(passPtr, &std::get<1>(parameter.second));
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
    
    void Material::setFloatParameter(uint16_t pass, const String &name, float value)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOAT, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valfloat = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatv3Parameter(uint16_t pass, const String &name, const kmVec3 &value)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATV3, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec3 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatv4Parameter(uint16_t pass, const String &name, const kmVec4 &value)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATV4, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valvec4 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatm3Parameter(uint16_t pass, const String &name, const kmMat3 &value)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATM3, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat3 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setFloatm4Parameter(uint16_t pass, const String &name, const kmMat4 &value)
    {
        lite3d_material_pass *passPtr = NULL;
        lite3d_shader_parameter *parameterPtr;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(String("Material \"") + getName() + "\" pass not found..");
        }

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_FLOATM4, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valmat4 = value;
        
        addParameter(passPtr, parameterPtr);
    }
    
    void Material::setSamplerTextureParameter(uint16_t pass, const String &name, Texture *texture)
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

        parameterPtr = getParameter(name, LITE3D_SHADER_PARAMETER_SAMPLER, LITE3D_FALSE, passPtr);
        SDL_assert_release(parameterPtr);

        /* update value */
        parameterPtr->parameter.valsampler.texture = texture->getPtr();
        
        addParameter(passPtr, parameterPtr);
    }

    float Material::getFloatParameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOAT)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valfloat;
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    kmVec3 Material::getFloatv3Parameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATV3)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valvec3;
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    kmVec4 Material::getFloatv4Parameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATV4)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valvec4;
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    kmMat3 Material::getFloatm3Parameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATM3)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valmat3;
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    kmMat4 Material::getFloatm4Parameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_FLOATM4)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");
            return std::get<1>(it->second).parameter.valmat4;
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    Texture *Material::getSamplerTextureParameter(const String &name) const
    {
        MaterialParameters::const_iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            if(std::get<1>(it->second).type != LITE3D_SHADER_PARAMETER_SAMPLER)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " type mismatch");

            if(!std::get<1>(it->second).parameter.valsampler.texture || !std::get<1>(it->second).parameter.valsampler.texture->userdata)
                throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not set");

            return static_cast<Texture *>(std::get<1>(it->second).parameter.valsampler.texture->userdata);
        }

        throw std::runtime_error(String("Material \"") + getName() + "\" parameter " + name + " not found..");
    }

    lite3d_shader_parameter *Material::getParameter(const String &name, 
        uint8_t type, uint8_t persist, lite3d_material_pass *passPtr)
    {
        MaterialParameters::iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            auto result = mMaterialParameters.insert(std::make_pair(name, std::make_tuple(passPtr, lite3d_shader_parameter())));
            it = result.first;
            
            lite3d_shader_parameter_init(&std::get<1>(it->second));
            
            std::get<0>(it->second) = passPtr;
            name.copy(std::get<1>(it->second).name, sizeof(std::get<1>(it->second).name)-1);
            std::get<1>(it->second).type = type;
            std::get<1>(it->second).persist = persist;
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
}

