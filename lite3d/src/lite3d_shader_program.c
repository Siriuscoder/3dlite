/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_shader_program.h>

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

int lite3d_shader_program_init(lite3d_shader_program *program)
{
    SDL_assert(program);
    
    program->statusString = NULL;
    program->success = 0;
    
    lite3d_misc_gl_error_stack_clean();
    if (glIsProgram(program->programID))
    {
        lite3d_shader_program_purge(program);
    }

    program->programID = glCreateProgram();
    if (lite3d_misc_check_gl_error())
    {
        glDeleteProgram(program->programID);
        return LITE3D_FALSE;
    }

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
    if (!glIsProgram(program->programID))
    {
        program->programID = glCreateProgram();
    }
    else
    {
        /*  if is program already created - relocate log string 
            before relink program */
        if (program->statusString)
        {
            lite3d_free(program->statusString);
            program->statusString = NULL;
        }
    }

    for (i = 0; i < count; ++i)
        glAttachShader(program->programID, shaders[i].shaderID);
    /* linking process */
    glLinkProgram(program->programID);

    lite3d_misc_check_gl_error();

    glGetProgramiv(program->programID, GL_LINK_STATUS, &isLinked);
    program->success = isLinked == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* validationg process */
    if (program->success)
    {
        glValidateProgram(program->programID);
        glGetProgramiv(program->programID, GL_VALIDATE_STATUS, &isValidated);
        program->success = isValidated == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;
    }

    /* get informaion log */
    glGetProgramiv(program->programID, GL_INFO_LOG_LENGTH, &maxLogLength);
    program->statusString = (char *) lite3d_malloc(maxLogLength);
    glGetProgramInfoLog(program->programID, maxLogLength, &maxLogLength, program->statusString);

    for (i = 0; i < count; ++i)
        glDetachShader(program->programID, shaders[i].shaderID);

    return program->success;
}

void lite3d_shader_program_purge(
    lite3d_shader_program *program)
{
    SDL_assert(program);

    if (glIsProgram(program->programID))
    {
        glDeleteProgram(program->programID);

        if (program->statusString)
        {
            lite3d_free(program->statusString);
            program->statusString = NULL;
        }
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

int32_t lite3d_shader_program_uniform_set(
    lite3d_shader_program *program, lite3d_shader_parameter *param,
    int32_t location)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);

    if (location < -1)
        return location;

    if (location == -1)
    {
        location = glGetUniformLocation(program->programID, param->name);
        if (location < 0)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: uniform %s not found in program %p",
                LITE3D_CURRENT_FUNCTION, param->name, (void *)program);
            return -2;
        }
    }

    switch (param->type)
    {
        case LITE3D_SHADER_PARAMETER_INT:
            glUniform1i(location, param->parameter.valint);
            break;
        case LITE3D_SHADER_PARAMETER_FLOAT:
            glUniform1f(location, param->parameter.valfloat);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATV3:
            glUniform3fv(location, 1, &param->parameter.valvec3.x);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATV4:
            glUniform4fv(location, 1, &param->parameter.valvec4.x);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATM3:
            glUniformMatrix3fv(location, 1, GL_FALSE, param->parameter.valmat3.mat);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATM4:
            glUniformMatrix4fv(location, 1, GL_FALSE, param->parameter.valmat4.mat);
            break;
        default:
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: unknown type for parameter %s",
                LITE3D_CURRENT_FUNCTION, param->name);
            return -2;
        }
    }

    return location;
}

int32_t lite3d_shader_program_sampler_set(
    lite3d_shader_program *program, lite3d_shader_parameter *param,
    int32_t location, uint16_t texUnit)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);

    if (location < -1)
        return location;

    if (param->type != LITE3D_SHADER_PARAMETER_SAMPLER)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: uniform %s not a sampler..",
            LITE3D_CURRENT_FUNCTION, param->name);
        return -2;
    }

    /* -1  mean what location unknown yet */
    if (location == -1)
    {
        location = glGetUniformLocation(program->programID, param->name);
        if (location < 0)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: sampler %s not found in program %p",
                LITE3D_CURRENT_FUNCTION, param->name, (void *)program);
            return -2;
        }
    }

    glUniform1i(location, texUnit);
    lite3d_texture_unit_bind(param->parameter.valsampler.texture, texUnit);
    return location;
}

void lite3d_shader_program_attribute_index(
    lite3d_shader_program *program, const char *name, int32_t location)
{
    SDL_assert(program);
    SDL_assert(glIsProgram(program->programID));

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
        "%s: bind attribute %s:%d for program %p",
        LITE3D_CURRENT_FUNCTION, name, location, (void *)program);

    glBindAttribLocation(program->programID, location, name);
}