/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_shader_program.h>

static int maxTextureSize;
static int maxTextureImageUnits;
static int maxCombinedTextureImageUnits;

int lite3d_shader_program_technique_init()
{
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);
    
    return LITE3D_TRUE;
}

int lite3d_shader_program_link(
    lite3d_shader_program *program, lite3d_shader *shaders, size_t count)
{
    uint32_t i;
    GLint isLinked, isValidated = 0;
    GLint maxLogLength = 0;

    SDL_assert(program);

    lite3d_misc_gl_error_stack_clean();
    if(!glIsProgram(program->programID))
    {
        program->programID = glCreateProgram();
    }
    else
    {
        /*  if is program already created - relocate log string 
            before relink program */
        lite3d_free(program->statusString);
        program->statusString = NULL;
    }

    for(i = 0; i < count; ++i)
        glAttachShader(program->programID, shaders->shaderID);
    /* linking process */
    glLinkProgram(program->programID);

    if(lite3d_misc_check_gl_error())
    {
        glDeleteProgram(program->programID);
        return LITE3D_FALSE;
    }

    glGetProgramiv(program->programID, GL_LINK_STATUS, &isLinked);
    program->success = isLinked == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* validationg process */
    if(program->success)
    {
        glValidateProgram(program->programID);     
        glGetProgramiv(program->programID, GL_VALIDATE_STATUS, &isValidated);
        program->success = isValidated == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;
    }

    /* get informaion log */
	glGetProgramiv(program->programID, GL_INFO_LOG_LENGTH, &maxLogLength);
    program->statusString = (char *)lite3d_malloc(maxLogLength);
    glGetProgramInfoLog(program->programID, maxLogLength, &maxLogLength, program->statusString);

    return program->success;
}

void lite3d_shader_program_purge(
    lite3d_shader_program *program)
{
    SDL_assert(program);

    if(glIsProgram(program->programID))
    {
        glDeleteProgram(program->programID);
        lite3d_free(program->statusString);
        program->statusString = NULL;
    }
}

void lite3d_shader_program_bind(
    lite3d_shader_program *program)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
    glUseProgram(program->programID);
}

void lite3d_shader_program_unbind(
    lite3d_shader_program *program)
{
    glUseProgram(0);
}

int lite3d_shader_program_uniform_set(
    lite3d_shader_program *program, lite3d_shader_parameter *param, 
    uint32_t location)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
}

int lite3d_shader_program_sampler_set(
    lite3d_shader_program *program, lite3d_shader_parameter *param, 
    uint32_t location, uint32_t texUnit)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
}