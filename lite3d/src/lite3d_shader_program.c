/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_shader_program.h>

typedef int (*lite3d_uniform_set_func)(lite3d_shader_program *, lite3d_shader_parameter_container *);

static lite3d_shader_program *gActProg = NULL;
static int gMaxGeometryOutputVertices = 0;
static int gMaxGeometryTotalOutputComponents = 0;
static int gMaxGeometryOutputComponents = 0;

static void lite3d_shader_program_get_log(lite3d_shader_program *program)
{
    GLint maxLogLength = 0;
    if (program->statusString)
    {
        lite3d_free(program->statusString);
        program->statusString = NULL;
    }

    glGetProgramiv(program->programID, GL_INFO_LOG_LENGTH, &maxLogLength);
    if (maxLogLength > 0)
    {
        program->statusString = (char *) lite3d_malloc(maxLogLength);
        glGetProgramInfoLog(program->programID, maxLogLength, &maxLogLength, program->statusString);
    }
}

int lite3d_shader_program_technique_init(void)
{
    if (lite3d_check_geometry_shader())
    {
        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &gMaxGeometryOutputVertices);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_GEOMETRY_OUTPUT_VERTICES: %d", gMaxGeometryOutputVertices);

        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &gMaxGeometryOutputComponents);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_GEOMETRY_OUTPUT_COMPONENTS: %d", gMaxGeometryOutputComponents);

        glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &gMaxGeometryTotalOutputComponents);
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS: %d", gMaxGeometryTotalOutputComponents);
    }

    return LITE3D_TRUE;
}

void lite3d_shader_program_get_limitations(int *maxGeometryOutputVertices, 
    int *maxGeometryOutputComponents, int *maxGeometryTotalOutputComponents)
{
    if (maxGeometryOutputVertices)
    {
        *maxGeometryOutputVertices = gMaxGeometryOutputVertices;
    }

    if (maxGeometryOutputComponents)
    {
        *maxGeometryOutputComponents = gMaxGeometryOutputComponents;
    }

    if (maxGeometryTotalOutputComponents)
    {
        *maxGeometryTotalOutputComponents = gMaxGeometryTotalOutputComponents;
    }
}

int lite3d_shader_program_init(lite3d_shader_program *program)
{
    SDL_assert(program);

    lite3d_misc_gl_error_stack_clean();
    memset(program, 0, sizeof(lite3d_shader_program));
    program->programID = glCreateProgram();
    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_shader_program_purge(program);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "shader program 0x%016llx created(%d)",
        (unsigned long long)program, program->programID);

    return LITE3D_TRUE;
}

int lite3d_shader_program_link(
    lite3d_shader_program *program, lite3d_shader *shaders, size_t count)
{
    uint32_t i;
    GLint isLinked = 0;
    SDL_assert(program);

    if (!glIsProgram(program->programID))
    {
        return LITE3D_FALSE;
    }

    lite3d_misc_gl_error_stack_clean();

    for (i = 0; i < count; ++i)
        glAttachShader(program->programID, shaders[i].shaderID);
    /* linking process */
    glLinkProgram(program->programID);

    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    glGetProgramiv(program->programID, GL_LINK_STATUS, &isLinked);
    program->success = isLinked == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* get informaion log */
    lite3d_shader_program_get_log(program);

    for (i = 0; i < count; ++i)
        glDetachShader(program->programID, shaders[i].shaderID);

    if (program->success)
    {
        if (program->statusString)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx link OK: %s", 
                program->programID, (unsigned long long)program, program->statusString);
        }
        else
        {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx link OK",
                program->programID, (unsigned long long)program);
        }
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx link FAILED: %s", 
            program->programID, (unsigned long long)program, program->statusString ? program->statusString : "No info");
    }

    return program->success;
}

int lite3d_shader_program_validate(
    lite3d_shader_program *program)
{
    GLint isValidated = 0;
    SDL_assert(program);

    /* validationg process */
    if (!program->success)
        return LITE3D_FALSE;
    if (program->validated)
        return LITE3D_TRUE;

    if (!glIsProgram(program->programID))
    {
        return LITE3D_FALSE;
    }

    glValidateProgram(program->programID);
    glGetProgramiv(program->programID, GL_VALIDATE_STATUS, &isValidated);
    program->validated = isValidated == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* get informaion log */
    lite3d_shader_program_get_log(program);

    if (program->validated)
    {
        if (program->statusString)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx validate OK: %s", 
                program->programID, (unsigned long long)program, program->statusString);
        }
        else
        {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx validate OK",
                program->programID, (unsigned long long)program);
        }
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "shader program(%d) 0x%016llx validate FAILED: %s", 
            program->programID, (unsigned long long)program, program->statusString ? program->statusString : "No info");
    }

    return program->validated;
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
    if (gActProg != program)
    {
        glUseProgram(program->programID);
        gActProg = program;
    }
}

void lite3d_shader_program_unbind(
    lite3d_shader_program *program)
{
    glUseProgram(0);
    gActProg = NULL;
}

static int lite3d_shader_program_sampler_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
    SDL_assert(p->parameter->parameter.texture);
    /* -1  mean what location unknown yet */
    if (p->location == -1)
    {
        p->location = glGetUniformLocation(program->programID, p->parameter->name);
        if (p->location < 0)
        {
            p->location = -2;
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: sampler '%s' not found in program(%d) 0x%016llx",
                LITE3D_CURRENT_FUNCTION, p->parameter->name, program->programID, (unsigned long long)program);
            return LITE3D_FALSE;
        }
    }
    else if (p->location == -2)
        return LITE3D_FALSE;
    
    if (p->binding < 0)
        p->binding = p->bindContext->textureBindingsCount++;
    
    glUniform1i(p->location, p->binding);
    lite3d_texture_unit_bind(p->parameter->parameter.texture, p->binding);
    return LITE3D_TRUE;
}

static int lite3d_shader_program_ssbo_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p)
{
#ifndef GLES
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
    SDL_assert(p->parameter->parameter.vbo);

    /* -1  mean what location unknown yet */
    if (p->location == -1)
    {
        p->location = glGetProgramResourceIndex(program->programID, 
            GL_SHADER_STORAGE_BLOCK, p->parameter->name);
        if (p->location < 0)
        {
            p->location = -2;
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: resource block '%s' not found in program(%d) 0x%016llx",
                LITE3D_CURRENT_FUNCTION, p->parameter->name, program->programID, (unsigned long long)program);
            return LITE3D_FALSE;
        }
    }
    else if (p->location == -2)
        return LITE3D_FALSE;
    
    if (p->binding < 0)
    {
        p->binding = p->bindContext->blockBindingsCount++;
        glShaderStorageBlockBinding(program->programID, p->location, p->binding);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, p->binding, p->parameter->parameter.vbo->vboID);
    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: Can`t set parameter '%s', SSBO is not supported in GLES, program 0x%016llx",
        LITE3D_CURRENT_FUNCTION, p->parameter->name, (unsigned long long)program);
    return LITE3D_FALSE;
#endif
}

static int lite3d_shader_program_ubo_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p)
{
#ifndef WITH_GLES2
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
    SDL_assert(p->parameter->parameter.vbo);

    /* -1  mean what location unknown yet */
    if (p->location == -1)
    {
        p->location = glGetUniformBlockIndex(program->programID, p->parameter->name);
        if (p->location < 0)
        {
            p->location = -2;
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: resource block '%s' not found in program(%d) 0x%016llx",
                LITE3D_CURRENT_FUNCTION, p->parameter->name, program->programID, (unsigned long long)program);
            return LITE3D_FALSE;
        }
    }
    else if (p->location == -2)
        return LITE3D_FALSE;
    
    if (p->binding < 0)
    {
        p->binding = p->bindContext->blockBindingsCount++;
        glUniformBlockBinding(program->programID, p->location, p->binding);
    }
    
    glBindBufferBase(GL_UNIFORM_BUFFER, p->binding, p->parameter->parameter.vbo->vboID);
    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "%s: Can`t set parameter '%s', UBO is not supported in GLES2, program 0x%016llx",
        LITE3D_CURRENT_FUNCTION, p->parameter->name, (unsigned long long)program);
    return LITE3D_FALSE;
#endif
}

static int lite3d_shader_program_simple_uniform_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p)
{
    SDL_assert(program);
    SDL_assert(program->success == LITE3D_TRUE);
    
    if (p->location == -1)
    {
        p->location = glGetUniformLocation(program->programID, p->parameter->name);
        if (p->location < 0)
        {
            p->location = -2; // not found 
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: uniform '%s' not found in program(%d) 0x%016llx",
                LITE3D_CURRENT_FUNCTION, p->parameter->name, program->programID, (unsigned long long)program);
            return LITE3D_FALSE;
        }
    }
    else if (p->location == -2) // not found in past
        return LITE3D_FALSE;

    switch (p->parameter->type)
    {
        case LITE3D_SHADER_PARAMETER_INT:
            glUniform1i(p->location, p->parameter->parameter.valint);
            break;
        case LITE3D_SHADER_PARAMETER_UINT:
#ifndef WITH_GLES2
            glUniform1ui(p->location, p->parameter->parameter.valuint);
#else
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: uniform '%s' type is not supported in program(%d) 0x%016llx",
                LITE3D_CURRENT_FUNCTION, p->parameter->name, program->programID, (unsigned long long)program);
            p->location = -2;
#endif
            break;
        case LITE3D_SHADER_PARAMETER_FLOAT:
            glUniform1f(p->location, p->parameter->parameter.valfloat);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATV3:
            glUniform3fv(p->location, 1, &p->parameter->parameter.valvec3.x);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATV4:
            glUniform4fv(p->location, 1, &p->parameter->parameter.valvec4.x);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATM3:
            glUniformMatrix3fv(p->location, 1, GL_FALSE, p->parameter->parameter.valmat3.mat);
            break;
        case LITE3D_SHADER_PARAMETER_FLOATM4:
            glUniformMatrix4fv(p->location, 1, GL_FALSE, p->parameter->parameter.valmat4.mat);
            break;
        default:
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: unknown type for parameter %s",
                LITE3D_CURRENT_FUNCTION, p->parameter->name);
            return LITE3D_FALSE;
        }
    }

    return LITE3D_TRUE;
}

static lite3d_uniform_set_func uniformsMethodsTable[] = {
    NULL,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_simple_uniform_set,
    lite3d_shader_program_sampler_set,
    lite3d_shader_program_ssbo_set,
    lite3d_shader_program_ubo_set
};

int lite3d_shader_program_uniform_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p)
{
    SDL_assert(p);
    SDL_assert(p->parameter);
    return uniformsMethodsTable[p->parameter->type](program, p);
}

void lite3d_shader_program_attribute_index(
    lite3d_shader_program *program, const char *name, int32_t location)
{
    SDL_assert(program);
    SDL_assert(glIsProgram(program->programID));

    SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
        "%s: bind attribute %s:%d for program(%d) 0x%016llx",
        LITE3D_CURRENT_FUNCTION, name, location, program->programID, (unsigned long long)program);

    glBindAttribLocation(program->programID, location, name);
}

int lite3d_shader_program_validate_current(void)
{
    if (!gActProg)
    {
        return LITE3D_FALSE;
    }

    return lite3d_shader_program_validate(gActProg);
}
