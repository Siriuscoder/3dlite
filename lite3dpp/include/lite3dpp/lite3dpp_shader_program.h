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
#pragma once

#include <lite3d/lite3d_shader_program.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ShaderProgram : public ConfigurableResource, public Noncopiable
    {
    public:

        ShaderProgram(const String &name, 
            const String &path, Main *main);

        inline lite3d_shader_program *getPtr()
        { return &mProgram; }

        static void setShaderVersion(const String &version);
        static void setFloatPrecision(const String &prec);
        static void addDefinition(const String &name, const String &value);

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        void loadShaders(stl<lite3d_shader>::vector &shaders);
        void unloadShaders(stl<lite3d_shader>::vector &shaders);
        void bindAttributeLocations();
        uint8_t determineShaderType(const String &filepath);
        void preprocessShaderCode(String &sourceCode);
        void optimizeShaderCode(String &sourceCode);
        static String createSourceHeader(uint8_t shaderType);

    private:

        lite3d_shader_program mProgram;
        static stl<String, String>::map mGlobalDefinitions;
        static String mShaderVersion;
        static String mFloatPrecision;
    };
}

