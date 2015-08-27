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
#include <algorithm>

#include <SDL_log.h>

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_shader_program.h>

namespace lite3dpp
{
    ShaderProgram::ShaderProgram(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::SHADER_PROGRAM)
    {}

    ShaderProgram::~ShaderProgram()
    {}

    void ShaderProgram::loadFromJsonImpl(const JsonHelper &helper)
    {
        stl<lite3d_shader>::vector shaders;
        shaders.reserve(2);

        try
        {
            loadShaders(shaders, LITE3D_SHADER_TYPE_VERTEX);
            loadShaders(shaders, LITE3D_SHADER_TYPE_FRAGMENT);
        }
        catch(std::exception &ex)
        {
            unloadShaders(shaders);
            throw ex;
        }

        try
        {
            if(!lite3d_shader_program_init(&mProgram))
                throw std::runtime_error("Shader program init failed..");

            mProgram.userdata = this;
            bindAttributeLocations();

            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                "Linking %s ...", getPath().c_str());

            if(!lite3d_shader_program_link(&mProgram, &shaders[0], shaders.size()))
                throw std::runtime_error(getPath() + " link: \"" + mProgram.statusString + "\"");
        }
        catch(std::exception &ex)
        {
            unloadShaders(shaders);
            unloadImpl();
            throw ex;
        }

        unloadShaders(shaders);
    }

    void ShaderProgram::unloadImpl()
    {
        lite3d_shader_program_purge(&mProgram);
    }

    void ShaderProgram::loadShaders(stl<lite3d_shader>::vector &shaders, uint8_t shaderType)
    {
        for(lite3dpp_string &source : getJson().getStrings(shaderType == LITE3D_SHADER_TYPE_VERTEX ? L"vs" : L"ps"))
        {
            shaders.resize(shaders.size()+1);

            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                "Compiling %s ...", source.c_str());

            if(!lite3d_shader_init(&shaders.back(), LITE3D_SHADER_TYPE_VERTEX))
                throw std::runtime_error("Shader init failed..");

            size_t sourceLen = 0;
            if(!lite3d_shader_compile(&shaders.back(), 
                (const char *)mMain->getResourceManager()->loadFileToMemory(source, &sourceLen), sourceLen))
                throw std::runtime_error(source + " compile: \"" + shaders.back().statusString + "\"");
        }
    }

    void ShaderProgram::unloadShaders(stl<lite3d_shader>::vector &shaders)
    {
        for(lite3d_shader &shader : shaders)
        {
            lite3d_shader_purge(&shader);
        }
    }

    void ShaderProgram::bindAttributeLocations()
    {
        int location = 0;
        for(lite3dpp_string &name : getJson().getStrings(L"AttributesLocation"))
        {
            lite3d_shader_program_attribute_index(&mProgram, name.c_str(), location);
            location++;
        }
    }
}

