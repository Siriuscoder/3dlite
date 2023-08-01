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

    uint8_t ShaderProgram::determineShaderType(const String &filepath)
    {
        if (filepath.find(".vs") != String::npos)
            return LITE3D_SHADER_TYPE_VERTEX;
        else if (filepath.find(".ps") != String::npos || filepath.find(".fs") != String::npos)
            return LITE3D_SHADER_TYPE_FRAGMENT;
        else if (filepath.find(".gs") != String::npos)
            return LITE3D_SHADER_TYPE_GEOMETRY;

        // will cause error in lite3d_shader_init
        return 0;
    }

    void ShaderProgram::loadShaders(stl<lite3d_shader>::vector &shaders)
    {
        for (String &source : getJson().getStrings(L"Sources"))
        {
            String sourcePath;
            String defPath;
            size_t flen;
            const char *rawdata;
            String shaderCode;

            shaders.resize(shaders.size()+1);
            sourcePath = source.substr(0, source.find_first_of(','));
            if (source.find_first_of(',') != String::npos)
                defPath = source.substr(source.find_first_of(',')+1);

           
            if (!lite3d_shader_init(&shaders.back(), determineShaderType(sourcePath)))
                LITE3D_THROW("Shader init failed..");

            // load definition source
            if (defPath.size() > 0)
            {
                rawdata = static_cast<const char *>(mMain->getResourceManager()->loadFileToMemory(defPath, &flen));
                shaderCode.append(rawdata, flen);
                shaderCode.append("\n");
            }

            // load main shader source
            rawdata = static_cast<const char *>(mMain->getResourceManager()->loadFileToMemory(sourcePath, &flen));
            shaderCode.append(rawdata, flen);

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Preprocessing \"%s\" ...", sourcePath.c_str());
            preprocessShaderCode(shaderCode);

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Optimizing \"%s\" ...", sourcePath.c_str());
            optimizeShaderCode(shaderCode);

            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Compiling \"%s\" ...", sourcePath.c_str());

            int32_t finalShaderLen[] = { static_cast<int32_t>(shaderCode.length()) };
            const char* finalShaderCode[] = { shaderCode.c_str() };
            if (!lite3d_shader_compile(&shaders.back(), 1, finalShaderCode, finalShaderLen))
                LITE3D_THROW(sourcePath << " compile: \"" << shaders.back().statusString << "\"");
            if (shaders.back().statusString && shaders.back().statusString[0] != 0)
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "%s compile: \"%s\"", sourcePath.c_str(), shaders.back().statusString);
            }
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

    void ShaderProgram::preprocessShaderCode(String &sourceCode)
    {
        auto includePos = sourceCode.find("#include");
        if (includePos != String::npos)
        {
            auto braceOpen = sourceCode.find_first_of("\"\n", includePos);
            if (braceOpen == String::npos || sourceCode[braceOpen] == '\n')
                LITE3D_THROW(getName() << ": include statement syntax error");

            auto braceClose = sourceCode.find_first_of("\"\n", braceOpen+1);
            if (braceClose == String::npos || sourceCode[braceClose] == '\n')
                LITE3D_THROW(getName() << ": include statement syntax error");

            auto includePath = sourceCode.substr(braceOpen + 1, braceClose - (braceOpen + 1));
            size_t flen;
            const char *rawdata = static_cast<const char *>(mMain->getResourceManager()->loadFileToMemory(includePath, &flen));
            sourceCode.erase(includePos, braceClose - includePos + 1);
            sourceCode.insert(includePos, rawdata, flen);
            // do it recursively
            preprocessShaderCode(sourceCode);
        }
    }

    void  ShaderProgram::optimizeShaderCode(String &sourceCode)
    {
        /* do nothing yet */
    }
}

