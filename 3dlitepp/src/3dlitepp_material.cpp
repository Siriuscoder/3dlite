/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#include <3dlitepp/3dlitepp_material.h>

namespace lite3dpp
{
    Material::Material(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::MATERIAL)
    {}

    Material::~Material()
    {}

    void Material::loadFromJsonImpl(const JsonHelper &helper)
    {

    }

    void Material::unloadImpl()
    {

    }
    
    uint16_t Material::addPass()
    {
        lite3d_material_pass *passPtr;
        
        if(!(passPtr = lite3d_material_add_pass(&mMaterial, mMaterial.passesSize+1)))
            throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" add pass failed..");
        
        for(MaterialParameters::value_type &parameter : mMaterialParameters)
        {
            /* meant that this parameter must used in all passes */
            if(std::get<0>(parameter.second) == NULL)
            {
                lite3d_material_pass_add_parameter(passPtr, &std::get<1>(parameter.second));
            }
        }
        
        mPasses.insert(std::make_pair(mMaterial.passesSize, passPtr));
        return mMaterial.passesSize;
    }
    
    void Material::removePass(uint16_t pass)
    {
        if(!lite3d_material_remove_pass(&mMaterial, pass))
            throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" remove pass failed..");
        mPasses.erase(pass);
    }
    
    void Material::setPassProgram(uint16_t pass, ShaderProgram *program)
    {
        lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" pass not found..");
        
        passPtr->program = program->getPtr();
    }
    
    ShaderProgram *Material::getPassProgram(uint16_t pass) const
    {
        const lite3d_material_pass *passPtr = lite3d_material_get_pass(&mMaterial, pass);
        if(!passPtr)
            throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" pass not found..");
        if(!passPtr->program)
            throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" program not specified..");
        
        return static_cast<ShaderProgram *>(passPtr->program->userdata);
    }
    
    void Material::setFloatParameter(uint16_t pass, const lite3dpp_string &name, float value)
    {
        lite3d_material_pass *passPtr = NULL;
        
        if(pass > 0)
        {
            passPtr = lite3d_material_get_pass(&mMaterial, pass);
            if(!passPtr)
                throw std::runtime_error(lite3dpp_string("Material \"") + getName() + "\" pass not found..");
        }
        
        MaterialParameters::iterator it;
        if((it = mMaterialParameters.find(name)) == mMaterialParameters.end())
        {
            auto result = mMaterialParameters.insert(std::make_pair(name, std::make_tuple(passPtr, lite3d_shader_parameter())));
            it = result.first;
            
            lite3d_shader_parameter_init(&std::get<1>(it->second));
            
            std::get<0>(it->second) = passPtr;
            name.copy(std::get<1>(it->second).name, sizeof(std::get<1>(it->second).name)-1);
            std::get<1>(it->second).type = LITE3D_SHADER_PARAMETER_FLOAT;
            std::get<1>(it->second).persist = LITE3D_FALSE;
        }
        
        /* update value */
        std::get<1>(it->second).parameter.valfloat = value;
        
        if(passPtr)
            lite3d_material_pass_add_parameter(passPtr, &std::get<1>(it->second));
        else
        {
            for(Passes::value_type &passEntry : mPasses)
                lite3d_material_pass_add_parameter(passEntry.second, &std::get<1>(it->second));
        }
    }
    
    void Material::setFloatv3Parameter(uint16_t pass, const lite3dpp_string &name, const kmVec3 &value)
    {
        
    }
    
    void Material::setFloatv4Parameter(uint16_t pass, const lite3dpp_string &name, const kmVec4 &value)
    {
        
    }
    
    void Material::setFloatm3Parameter(uint16_t pass, const lite3dpp_string &name, const kmMat3 &value)
    {
        
    }
    
    void Material::setFloatm4Parameter(uint16_t pass, const lite3dpp_string &name, const kmMat4 &value)
    {
        
    }
    
    void Material::setSamplerTextureParameter(uint16_t pass, const lite3dpp_string &name, Texture *texture)
    {
        
    }
}

