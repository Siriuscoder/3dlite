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
#pragma once

#include <lite3d/lite3d_shader_program.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_json_helper.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ShaderProgram : public JsonResource, public NoncopiableResource
    {
    public:

        ShaderProgram(const String &name, 
            const String &path, Main *main);

        ~ShaderProgram();

        inline lite3d_shader_program *getPtr()
        { return &mProgram; }

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override;
        virtual void unloadImpl() override;

    private:

        void loadShaders(stl<lite3d_shader>::vector &shaders);
        void unloadShaders(stl<lite3d_shader>::vector &shaders);
        void bindAttributeLocations();

    private:

        lite3d_shader_program mProgram;
    };
}

