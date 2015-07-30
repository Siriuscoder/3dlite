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

    }

    void ShaderProgram::unloadImpl()
    {

    }
}

