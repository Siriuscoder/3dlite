/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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

#include <array>

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
            const String &path, Main &main);

        inline lite3d_shader_program *getPtr()
        { return &mProgram; }

        static void setShaderVersion(const String &version);
        static void setFloatPrecision(const String &prec);
        static void addGlobalDefinition(const String &name, const String &value);
        /**
         * Choose balanced power-of-two local sizes within the resolution and GPU limits.
         * Axes with resolution 1 stay at 1. The product never exceeds the invocation limit.
         * This is a heuristic, not a performance guarantee; ceil-divide the resolution by
         * the returned sizes for dispatch and guard out-of-bounds invocations in the shader.
         * Requires initialized shader technique limits and nonzero resolution on all axes.
         */
        static std::array<uint32_t, 3> getOptimalWorkGroupSize(uint32_t width, uint32_t height = 1, uint32_t depth = 1);
        void addLocalDefinition(const String &name, const String &value);

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
        String createSourceHeader(uint8_t shaderType);

    private:

        lite3d_shader_program mProgram;
        stl<String, String>::map mLocalDefinitions;
        static stl<String, String>::map mGlobalDefinitions;
        static String mShaderVersion;
        static String mFloatPrecision;
    };
}
