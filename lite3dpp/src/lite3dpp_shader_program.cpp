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
#include <algorithm>

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_shader_program.h>

namespace lite3dpp
{
    ShaderProgram::ShaderProgram(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::SHADER_PROGRAM)
    {}

    ShaderProgram::~ShaderProgram()
    {}

    void ShaderProgram::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        stl<lite3d_shader>::vector shaders;
        shaders.reserve(2);

        try
        {
            loadShaders(shaders);
        }
        catch(std::exception &)
        {
            unloadShaders(shaders);
            throw;
        }

        try
        {
            if(!lite3d_shader_program_init(&mProgram))
                LITE3D_THROW("Shader program init failed..");

            mProgram.userdata = this;
            bindAttributeLocations();

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Linking \"%s\" ...", getPath().c_str());

            if(!lite3d_shader_program_link(&mProgram, &shaders[0], shaders.size()))
                LITE3D_THROW(getPath() << " link: \"" << mProgram.statusString << "\"");
        }
        catch(std::exception &)
        {
            unloadShaders(shaders);
            unloadImpl();
            throw;
        }

        unloadShaders(shaders);
    }

    void ShaderProgram::unloadImpl()
    {
        lite3d_shader_program_purge(&mProgram);
    }

    void ShaderProgram::loadShaders(stl<lite3d_shader>::vector &shaders)
    {
        for(String &source : getJson().getStrings(L"Sources"))
        {
            String sourcePath;
            String defPath;
            size_t slen;
            stl<const char *>::vector sources;
            stl<int32_t>::vector sourcesLen;

            shaders.resize(shaders.size()+1);
            sourcePath = source.substr(0, source.find_first_of(','));
            if (source.find_first_of(',') != String::npos)
                defPath = source.substr(source.find_first_of(',')+1);

            if(!lite3d_shader_init(&shaders.back(), 
                sourcePath.find(".vs") != String::npos ? LITE3D_SHADER_TYPE_VERTEX : LITE3D_SHADER_TYPE_FRAGMENT))
                LITE3D_THROW("Shader init failed..");

            // load definition source
            if (defPath.size() > 0)
            {
                sources.push_back((const char *)mMain->getResourceManager()->loadFileToMemory(defPath, &slen));
                sourcesLen.push_back(slen);
            }

            // load main shader source
            sources.push_back((const char *)mMain->getResourceManager()->loadFileToMemory(sourcePath, &slen));
            sourcesLen.push_back(slen);

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Compiling \"%s\" ...", sourcePath.c_str());

            if(!lite3d_shader_compile(&shaders.back(), sources.size(),
                &sources[0], &sourcesLen[0]))
                LITE3D_THROW(sourcePath << " compile: \"" << shaders.back().statusString << "\"");
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
        for(String &name : getJson().getStrings(L"AttributesOrder"))
        {
            lite3d_shader_program_attribute_index(&mProgram, name.c_str(), location);
            location++;
        }
    }
}

